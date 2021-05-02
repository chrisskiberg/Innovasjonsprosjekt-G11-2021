from django.shortcuts import render
from .models import SensorData
from django.middleware import csrf
from django.http import QueryDict, JsonResponse
from django.http import HttpResponse, QueryDict
import requests
from django.core import serializers
from json import dumps 
from django.core.serializers.json import DjangoJSONEncoder
import datetime

# import keyboard_sensor.py 
# >>> from elsysapp.models import SensorData


# Create your views here.
def index(request):
    # print("Dette blir printa i terminalen")
    # context = {}
    # all_sensor_data = SensorData.objects.all() #Henter ut all sensordata fra databasen. 
    # context['all_sensor_data'] = all_sensor_data #Legger sensordata til som en variabel som kan brukes i Template. 
    return render(request, "elsysapp/index.html", {})

def contact(request):
    return render(request, "elsysapp/contact.html", {})

def about(request):
    return render(request, "elsysapp/about.html", {})

def login(request):
    return render(request, "elsysapp/login.html", {})

def product(request):
    return render(request, "elsysapp/product.html", {})

def eldreokter(request):
    return render(request, "elsysapp/eldreokter.html", {})


def get_sensor_data(request):
    if request.method == "POST": 
        accdata =  QueryDict(request.body) # Gjør data fra request om til en dictionary
        xaccdata = accdata['xdata'] # Lagrer sensorverdien til requesten
        yaccdata = accdata['ydata'] # Lagrer sensorverdien til requesten
        zaccdata = accdata['zdata'] # Lagrer sensorverdien til requesten
        new_data = SensorData(xData=xaccdata, yData=yaccdata, zData=zaccdata)
        new_data.save()
        return HttpResponse("")
    elif request.method == "GET":
        """Dette MÅ være med! Sikkerhetsgreier."""
        csrf.get_token(request)
        return HttpResponse("")





# def dbdata(request):
#     json_serializer = serializers.get_serializer("json")()
#     db_data = json_serializer.serialize(SensorData.objects.all(), ensure_ascii=False) 
#     return direct_to_template(request, 'index.html', {'db_data': db_data})






def myconverter(o):
    if isinstance(o, datetime.datetime):
        return o.__str__()


def send_dictionary(request): 
    objects = SensorData.objects.all()

    # xdict = { "xdata" : [ x for x in objects.values_list("xData") ]}
    # ydict = { "ydata" : [ x for x in objects.values_list("yData") ]}
    # zdict = { "zdata" : [ x for x in objects.values_list("zData") ]}
    timedict = { "date" : [x[0].__str__() for x in objects.values_list("timestamp")]}
    # print([datetime.date(x[0].year,x[0].month,x[0].day)  for x in objects.values_list("timestamp")])
    # xdataJSON = dumps(xdict) 
    # ydataJSON = dumps(ydict) 
    # zdataJSON = dumps(zdict) 
    timeJSON = dumps(timedict)

    return render(request, 'elsysapp/dbdata.html', {'timestamp':timeJSON}) 




def dbdata_sanntid(request, ids):
    ids=ids.replace('_', ' ')
    ids_arr=set()
    [ids_arr.add(int(s)) for s in ids.split() if s.isdigit()]

    # arr_int_string=[s.replace('-', ' ') for s in ids.split() if not s.isdigit()]
    # arr_int=[]
    # print(arr_int)
    # for el in arr_int_string:
    #     [arr_int.append(s) for s in el if s.isdigit()]
    # for i in range(0,len(arr_int), 2):
    #     for j in range(int(arr_int[i]), int(arr_int[i+1])+1,1):
    #         ids_arr.add(j)
    # ids_arr=list(ids_arr)

    arr_int_string=[s.replace('-', ' ') for s in ids.split() if not s.isdigit()]
    #print(arr_int_string)
    arr_int=[]
    for i in range(len(arr_int_string)):
        [arr_int.append(int(s)) for s in arr_int_string[i].split(" ") if s.isdigit()]
    #print(arr_int)
    for i in range(0,len(arr_int), 2):
        for j in range(int(arr_int[i]), int(arr_int[i+1])+1,1):
            ids_arr.add(j)
    ids_arr=list(ids_arr)
    print(ids_arr)



    objects = SensorData.objects.filter(id__in=ids_arr)
    xdict = { "xdata" : [ x for x in objects.values_list("xData") ]}
    ydict = { "ydata" : [ x for x in objects.values_list("yData") ]}
    zdict = { "zdata" : [ x for x in objects.values_list("zData") ]}
    timedict = { "date" : [ x[0].__str__() for x in objects.values_list("timestamp") ]}

    xdataJSON = dumps(xdict) 
    ydataJSON = dumps(ydict) 
    zdataJSON = dumps(zdict) 
    timeJSON = dumps(timedict)
    print(xdataJSON, '\n', '\n')
    print(timeJSON)

    # field_object = SensorData._meta.get_field("data")
    # field_value = field_object.value_from_object(objects_last)
    # datadict = { "data" : [field_value]}
    iddict = { "id" : [x for x in ids_arr]}
    idJSON = dumps(iddict) 
    return render(request, 'elsysapp/dbdata_sanntid.html', {'xdata': xdataJSON, 'ydata': ydataJSON, 'zdata':zdataJSON, 'timestamp':timeJSON, 'id':idJSON,}) 


def sanntid(request, ids):
    objects_last = SensorData.objects.last()

    ids=ids.replace('_', ' ')
    ids_arr=set()
    [ids_arr.add(int(s)) for s in ids.split() if s.isdigit()]
    ids_arr.add(objects_last.id)


    arr_int_string=[s.replace('-', ' ') for s in ids.split() if not s.isdigit()]
    arr_int=[]
    for el in arr_int_string:
        [arr_int.append(s) for s in el if s.isdigit()]
    for i in range(0,len(arr_int), 2):
        for j in range(int(arr_int[i]), int(arr_int[i+1])+1):
            ids_arr.add(j)
    ids_arr=list(ids_arr)
    # print(ids_arr)


    objects = SensorData.objects.filter(id__in=ids_arr)
    xdict = { "xdata" : [ x for x in objects.values_list("xData") ]}
    ydict = { "ydata" : [ x for x in objects.values_list("yData") ]}
    zdict = { "zdata" : [ x for x in objects.values_list("zData") ]}
    timedict = { "date" : [ x[0].__str__() for x in objects.values_list("timestamp") ]}

    xdataJSON = dumps(xdict) 
    ydataJSON = dumps(ydict) 
    zdataJSON = dumps(zdict) 
    timeJSON = dumps(timedict)
    iddict = { "id" : [x for x in ids_arr]}
    idJSON = dumps(iddict) 
    return render(request, 'elsysapp/graphtest.html', {'xdata': xdataJSON, 'ydata': ydataJSON, 'zdata':zdataJSON, 'timestamp':timeJSON, 'id':idJSON,}) 


# def sanntid_new(request, ids):
#     objects_last = SensorData.objects.last()

#     ids=ids.replace('-', ' ')
#     ids_arr=set()
#     [ids_arr.add(int(s)) for s in ids.split() if s.isdigit()]
#     ids_arr.add(objects_last.id)
#     ids_arr=list(ids_arr)
#     print(ids_arr)

#     objects = SensorData.objects.filter(id__in=ids_arr)
#     datadict = { "data" : [ x for x in objects.values_list("data") ]}
#     dataJSON = dumps(datadict) 

#     # field_object = SensorData._meta.get_field("data")
#     # field_value = field_object.value_from_object(objects_last)
#     # datadict = { "data" : [field_value]}
#     iddict = { "id" : [x for x in ids_arr]}
#     dataJSON = dumps(datadict) 
#     idJSON = dumps(iddict) 
#     return render(request, 'elsysapp/graphtest.html', {'data': dataJSON, 'id':idJSON,}) 


def sanntid_first(request):     
    # objects_last = SensorData.objects.last()
    objects = SensorData.objects.filter(id=SensorData.objects.last().id)

    xdict = { "xdata" : [ x for x in objects.values_list("xData") ]}
    ydict = { "ydata" : [ x for x in objects.values_list("yData") ]}
    zdict = { "zdata" : [ x for x in objects.values_list("zData") ]}
    timedict = { "date" : [ x[0].__str__() for x in objects.values_list("timestamp") ]}
    iddict = { "id" : [SensorData.objects.last().id]}

    xdataJSON = dumps(xdict) 
    ydataJSON = dumps(ydict) 
    zdataJSON = dumps(zdict) 
    timeJSON = dumps(timedict)
    idJSON = dumps(iddict) 
    
    return render(request, 'elsysapp/graphtest_first.html', {'xdata': xdataJSON, 'ydata': ydataJSON, 'zdata':zdataJSON, 'timestamp':timeJSON, 'id':idJSON,}) 


def demonstrasjon(request):
    objects_last = SensorData.objects.last()
    objects = SensorData.objects.filter(id=objects_last.id)

    x_tuple_raw=[ x for x in objects.values_list("xData") ]
    y_tuple_raw=[ x for x in objects.values_list("yData") ]
    z_tuple_raw=[ x for x in objects.values_list("zData") ]
    # print(x_tuple_raw[0][0])
    # print(y_tuple_raw[0][0])
    # print(z_tuple_raw[0][0])
    x_arr_raw=x_tuple_raw[0][0].split(",")
    y_arr_raw=y_tuple_raw[0][0].split(",")
    z_arr_raw=z_tuple_raw[0][0].split(",")
    for i in range(len(x_arr_raw)):
        if x_arr_raw[i]=="":
            x_arr_raw[i]=0
        else:
            x_arr_raw[i]=int(x_arr_raw[i], 16)*9.81/4096

    for i in range(len(y_arr_raw)):
        if y_arr_raw[i]=="":
            y_arr_raw[i]=0
        else:
            y_arr_raw[i]=int(y_arr_raw[i], 16)*9.81/4096

    for i in range(len(z_arr_raw)):
        if z_arr_raw[i]=="":
            z_arr_raw[i]=0
        else:
            z_arr_raw[i]=int(z_arr_raw[i], 16)*9.81/4096

    # print(x_arr_raw)
    # print(y_arr_raw)
    # print(z_arr_raw)

    vel_vec_arr=[[0,0,0]]
    k=1
    # ax3.scatter(vel_vec_arr[0][0], vel_vec_arr[0][1], vel_vec_arr[0][2], color=colors[i].hex)
    # finner farte i x-, y- og z-retning
    for i in range(0, len(x_arr_raw)-1): # 
        vel_vec_arr.append([vel_vec_arr[k-1][0]+x_arr_raw[i]/62.5, vel_vec_arr[k-1][1]+y_arr_raw[i]/62.5, vel_vec_arr[k-1][2]+z_arr_raw[i]/62.5])
        k+=1

    vel_x=[]
    vel_y=[]
    vel_z=[]
    for i in range(0, len(x_arr_raw)-1):
        vel_x.append(vel_vec_arr[i][0])
        vel_y.append(vel_vec_arr[i][1])
        vel_z.append(vel_vec_arr[i][2])

    # print(vel_x)
    # print(vel_y)
    # print(vel_z)

    x_acc_dict = { "xAcc" : x_arr_raw}
    y_acc_dict = { "yAcc" : y_arr_raw}
    z_acc_dict = { "zAcc" : z_arr_raw}
    x_vel_dict = { "xVel" : vel_x}
    y_vel_dict = { "yVel" : vel_y}
    z_vel_dict = { "zVel" : vel_z}

    timedict = { "date" : [ x[0].__str__() for x in objects.values_list("timestamp") ]}

    xAccJSON = dumps(x_acc_dict) 
    yAccJSON = dumps(y_acc_dict) 
    zAccJSON = dumps(z_acc_dict)    
    xVelJSON = dumps(x_vel_dict) 
    yVelJSON = dumps(y_vel_dict) 
    zVelJSON = dumps(z_vel_dict) 

    timeJSON = dumps(timedict)

    # iddict = { "id" : [objects_last.id]}
    # idJSON = dumps(iddict) 


    # field_object = SensorData._meta.get_field("data")
    # field_value = field_object.value_from_object(objects_last)
    # datadict = { "data" : [field_value]}
    iddict = { "id" : [objects_last.id]} 
    idJSON = dumps(iddict) 
    return render(request, 'elsysapp/demonstrasjon.html', {'xAcc': xAccJSON, 'yAcc': yAccJSON, 'zAcc':zAccJSON,'xVel': xVelJSON, 'yVel': yVelJSON, 'zVel':zVelJSON, 'timestamp':timeJSON, 'id':idJSON}) 


