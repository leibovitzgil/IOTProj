
import os
import glob
import subprocess
import calendar
import time
import urllib.request
import json


class DBuploader:

    def __init__(self,urlpath):
        self.urlpath = urlpath

    def uploadData(self,data_to_upload):  
        url = 'https://sisdb-20a34.firebaseio.com/a.json'

        postdata = {
            'Time': str(calendar.timegm(time.gmtime())),
            'LightRate': data_to_upload
        }   


        headers = {'Content-Type':'application/json'}
        data = json.dumps(postdata)
        data = data.encode('utf-8')
        req = urllib.request.Request(url,data=data,headers=headers)
        response = urllib.request.urlopen(req)
