from django.shortcuts import render, redirect
from django.contrib.auth import authenticate, login, logout
from django.contrib.auth.decorators import login_required
from django.contrib.auth.models import User
from django.contrib import messages
from django.utils import timezone
from datetime import timedelta
from django.http import HttpResponse

# Import de tes modèles
from .models import Measurement, Device, DevicePairing

def register_page(request):
    """Page d'inscription"""
    if request.user.is_authenticated:
        return redirect('/main/')
    error = None
    if request.method == "POST":
        username = request.POST.get('username', '').strip()
        email = request.POST.get('email', '').strip()
        password = request.POST.get('password', '')
        password_confirm = request.POST.get('password_confirm', '')
        
        if not username or len(username) < 3:
            error = "Nom d'utilisateur trop court (min 3)."
        elif User.objects.filter(username=username).exists():
            error = "Ce pseudo est déjà pris."
        elif password != password_confirm:
            error = "Les mots de passe ne correspondent pas."
        else:
            User.objects.create_user(username=username, email=email, password=password)
            messages.success(request, "Compte créé avec succès ! Connectez-vous.")
            return redirect('/login/')
    return render(request, "register.html", {"error": error})

def login_page(request):
    """Étape 1 : Connexion Pseudo + Mot de passe fixe"""
    if request.user.is_authenticated:
        return redirect('/verify-kit/')
    
    error = None
    if request.method == "POST":
        username = request.POST.get('username', '').strip()
        password = request.POST.get('password', '')
        user = authenticate(username=username, password=password)
        
        if user is not None:
            login(request, user)
            request.session['kit_verified'] = False
            return redirect('/verify-kit/')
        else:
            error = "Identifiants incorrects."
            
    return render(request, "login.html", {"error": error})

@login_required
def verify_kit_page(request):
    """Étape 2 : Vérification du code temporaire de l'ESP32"""
    error = None
    # Si l'utilisateur n'a pas de kit, on l'envoie en ajouter un
    if not Device.objects.filter(owner=request.user).exists():
        return redirect('/add-device/')

    if request.method == "POST":
        code_saisi = request.POST.get('temp_code', '').strip()
        is_valid = DevicePairing.objects.filter(
            device__owner=request.user, 
            pairing_code=code_saisi
        ).exists()
        
        if is_valid:
            request.session['kit_verified'] = True
            return redirect('/main/')
        else:
            error = "Code invalide. Regardez votre MQTTX ou votre kit."

    return render(request, "verify_kit.html", {"error": error})

@login_required(login_url='/login/')
def main_page(request):
    """Étape 3 : Landing Page (Dashboard)"""
    if not request.session.get('kit_verified', False):
        return redirect('/verify-kit/')

    # Données récentes pour les graphiques
    data = Measurement.objects.filter(
        sensor__device__owner=request.user
    ).select_related('sensor').order_by('-timestamp')
    
    sensors = {}
    for m in data[:20]: # On prend les dernières pour les tuiles
        st = m.sensor.sensor_type
        if st not in sensors:
            sensors[st] = {'latest': float(m.value), 'time': m.timestamp}
    
    return render(request, 'main.html', {
        'username': request.user.username,
        'sensor_data': sensors,
        'history_data': data[:100]
    })

@login_required
def add_device(request):
    """Lier un appareil au compte via son code actuel"""
    error = None
    if request.method == "POST":
        code_saisi = request.POST.get('pairing_code', '').strip()
        try:
            pairing_info = DevicePairing.objects.get(pairing_code=code_saisi)
            device = pairing_info.device
            device.owner = request.user
            device.save()
            request.session['kit_verified'] = True
            return redirect('/main/')
        except DevicePairing.DoesNotExist:
            error = "Code de pairing introuvable."
    return render(request, 'add_device.html', {'error': error})

def logout_page(request):
    logout(request)
    return redirect('/login/')

def mqtt_page(request):
    return HttpResponse("Flux MQTT actif en arrière-plan")