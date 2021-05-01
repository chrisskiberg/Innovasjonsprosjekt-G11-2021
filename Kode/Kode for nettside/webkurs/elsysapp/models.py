from django.db import models

# Create your models here.
class SensorData(models.Model):
    xData=models.CharField(max_length=15000)
    yData=models.CharField(max_length=15000)
    zData=models.CharField(max_length=15000)
    timestamp = models.DateTimeField(auto_now_add=True) # Lager et datofelt 
    
    def __str__(self): # Funksjon som skriver ut dataene
        return "xData: {}, yData: {}, zData: {}, timestamp: {}".format(self.xData, self.yData, self.zData, self.timestamp)

