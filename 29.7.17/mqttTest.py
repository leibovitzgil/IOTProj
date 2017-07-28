from flask import Flask
from flask import jsonify
from flask import request
import paho.mqtt.client as mqtt
import pymongo
import json
import datetime

app = Flask(__name__)
topic = "sensor"


client = pymongo.MongoClient("localhost", 27017)
db = client['mqtt-db']
mqtt_collection = db['mqtt-collection']


# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe(topic)

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    data_str = str(msg.payload)
    data = json.loads(data_str)

    print data_str
    print data
    data["date"] = datetime.datetime.now()
    mqtt_collection.save(data)
    print(msg.topic+" "+str(msg.payload))


@app.route("/get_obs")
def get_obs():
    res = []
    for row in mqtt_collection.find():
        del row['_id']
        res.append(row)
    return jsonify(res)

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect("10.0.0.6", 1883, 60)

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
client.loop_forever()