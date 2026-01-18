from django.contrib import admin
from django.urls import path
from django.shortcuts import redirect
# Assure-toi que le nom ici est EXACTEMENT le même que dans views.py
from api.views import (
    login_page, 
    register_page, 
    logout_page, 
    main_page, 
    add_device, 
    verify_kit_page,  # <--- Vérifie bien ce nom
    mqtt_page
)

urlpatterns = [
    path('admin/', admin.site.urls),
    path('', lambda request: redirect('login/'), name='home'),
    
    path('login/', login_page, name='login'),
    path('register/', register_page, name='register'),
    path('logout/', logout_page, name='logout'),
    path('verify-kit/', verify_kit_page, name='verify_kit'), 
    path('main/', main_page, name='main'),
    path('mqtt/', mqtt_page, name='mqtt'),
    path('add-device/', add_device, name='add_device'),
]