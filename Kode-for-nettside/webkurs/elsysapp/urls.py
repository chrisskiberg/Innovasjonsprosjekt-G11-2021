from django.contrib import admin
from django.urls import path
from .views import index, get_sensor_data, send_dictionary, sanntid,sanntid_first # dbdata #, aksel Relativ import av viewsfunksjonen
from .views import contact, about, login, product, eldreokter, dbdata_sanntid, demonstrasjon


appname = "elsysapp"
urlpatterns = [
    path('', index, name='index'),
    path('contact/', contact, name="contact"),
    path('about/', about, name="about"),
    path('login/', login, name="login"),
    path('product/', product, name="product"),
    path('eldreokter/', eldreokter, name="eldreokter"),


    # path('chart/', chart, name='chart'),
    path('sensor/', get_sensor_data, name='sensor'),
    path('dbdata/', send_dictionary, name='dbdata'),
    path('dbdata/<slug:ids>/', dbdata_sanntid, name='dbdata_sanntid'),
    # path('profile/', profile),
    path('sanntid/', sanntid_first, name='sanntid_first'),
    path('sanntid/<slug:ids>/', sanntid, name='sanntid'),
    # path('demonstrasjon/', demonstrasjon, name='demonstrasjon'),

    # path('^index$', 'dbdata.views.index'), 

    # 127.0.0.1:8000/elsysapp/sensor/
    # path("aksel/", aksel, name='aksel') kommer inn
]