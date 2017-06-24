from flask import Flask
from flask import jsonify
from flask import request
import pymongo
import time
import datetime
import json

app = Flask(__name__)


client = pymongo.MongoClient("localhost", 27017)
db = client['obs-db']
obs_collection = db['obs-collection']


@app.route("/obs")
def obs():
    data_str = request.args.get("data")
    data = json.loads(data_str)
    data["date"] = datetime.datetime.now()
    obs_collection.save(data)
    return "success"


@app.route("/get_obs")
def get_obs():
    res = []
    for row in obs_collection.find():
        del row['_id']
        res.append(row)
    return jsonify(res)


@app.route("/delete_all")
def delete_all():
    res = obs_collection.delete_many({})
    return jsonify({"deleted": res.deleted_count})

if __name__ == "__main__":
    app.run(host="0.0.0.0", debug=True)
