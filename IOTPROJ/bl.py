import datetime
import pymongo
import time

client = pymongo.MongoClient("localhost", 27017)
db = client['obs-db']
obs_collection = db['obs-collection']


#create tree

attributes = ["temperature", "humidity", "light"]
target = "irrigate"


def should_water():
    last_obs = obs_collection.find().sort({"date": -1}).limit(1)
    row = [last_obs[field] for field in attributes if not field == target]
    #return respond by tree
    return True


dt = datetime.timedelta(seconds=10)

if __name__ == "__main__":
    last_acted = None
    while True:
        now = datetime.datetime.now()
        if last_acted is None or datetime.datetime.now() - last_acted > dt:
            last_acted = now
            res = should_water()
            print "YES" if res else "NO"
        time.sleep(1)
