from django.apps import AppConfig
import threading
import os

def start_mqtt_subscriber():
    """Démarre le subscriber MQTT dans un thread séparé"""
    import paho.mqtt.client as mqtt
    import json
    
    # --- CALLBACK : Connexion établie ---
    def on_connect(client, userdata, flags, rc, properties=None):
        print(f"✅ Django connecté au Broker MQTT (Code {rc})")
        # On s'abonne aux deux flux : capteurs et pairing
        # Le '+' est un joker qui capture n'importe quel ID MAC
        client.subscribe("ynov/home/+/sensors")
        client.subscribe("ynov/appairage/+/code")
        print("📡 Abonnements activés sur broker.emqx.io")

    # --- CALLBACK : Message reçu ---
    def on_message(client, userdata, msg):
        # Importations locales pour éviter les erreurs de chargement Django
        from api.models import Device, Sensor, Measurement, DevicePairing
        
        # On extrait l'ID MAC du topic (ex: ynov/home/560b65f4/sensors)
        topic_parts = msg.topic.split('/')
        if len(topic_parts) < 3:
            return
        device_mac = topic_parts[2]

        # 1. RÉCUPÉRER OU CRÉER LE DEVICE (Auto-provisioning)
        device, created = Device.objects.get_or_create(
            mqtt_client_id=device_mac,
            defaults={
                'name': f"ESP32 {device_mac}",
                'status': 'online',
                'ip_address': '0.0.0.0'
            }
        )
        if created:
            print(f"🆕 Nouveau kit détecté en base de données : {device_mac}")

        # 2. LOGIQUE APPAIRAGE
        if "appairage" in msg.topic:
            try:
                pairing_code = msg.payload.decode()
                pairing, _ = DevicePairing.objects.get_or_create(device=device)
                pairing.pairing_code = pairing_code
                pairing.save()
                print(f"🔑 Code Pairing [{pairing_code}] reçu pour {device_mac}")
            except Exception as e:
                print(f"❌ Erreur enregistrement pairing : {e}")

        # 3. LOGIQUE CAPTEURS
        elif "sensors" in msg.topic:
            try:
                payload = json.loads(msg.payload.decode())
                data_str = payload['msg']  # Format attendu : "25.5°C, 40.0%, N"

                # Nettoyage des unités et découpage
                clean_data = data_str.replace('°C', '').replace('%', '')
                parts_data = [p.strip() for p in clean_data.split(',')]
                
                # Extraction des valeurs
                temp = float(parts_data[0])
                hum = float(parts_data[1])
                # Y = Présence (1.0), N = Absence (0.0)
                presence = 1.0 if parts_data[2] == 'Y' else 0.0

                # Enregistrement groupé en base de données
                sensor_map = {
                    'temperature': temp,
                    'humidity': hum,
                    'presence': presence
                }

                for s_type, val in sensor_map.items():
                    sensor, _ = Sensor.objects.get_or_create(
                        device=device, 
                        sensor_type=s_type,
                        defaults={'name': f"{s_type} de {device_mac}"}
                    )
                    Measurement.objects.create(sensor=sensor, value=val)
                
                print(f"💾 {device_mac} -> T:{temp}°C | H:{hum}% | P:{presence}")

            except Exception as e:
                print(f"❌ Erreur traitement données pour {device_mac} : {e}")

    def run_mqtt_client():
        """Configuration et boucle du client MQTT"""
        client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION1)
        
        # On lie les fonctions de rappel
        client.on_connect = on_connect
        client.on_message = on_message
        
        try:
            # Connexion au broker public (vérifie que ton PC a internet)
            client.connect("broker.emqx.io", 1883, 60)
            print("🚀 Subscriber MQTT en attente de messages...")
            client.loop_forever() # Boucle infinie pour le thread
        except Exception as e:
            print(f"❌ Échec de connexion au broker : {e}")

    # Lancement du client dans un thread pour ne pas bloquer le serveur Web
    mqtt_thread = threading.Thread(target=run_mqtt_client, daemon=True)
    mqtt_thread.start()


class ApiConfig(AppConfig):
    default_auto_field = 'django.db.models.BigAutoField'
    name = 'api'
    
    def ready(self):
        """Déclenchement au démarrage de Django"""
        # RUN_MAIN permet d'éviter que le thread se lance deux fois 
        # (à cause du système de rechargement automatique de Django)
        if os.environ.get('RUN_MAIN') == 'true':
            start_mqtt_subscriber()