# ****************************************************************************************************
# API Commands
#
# Roy C. Davies, 2023
# ****************************************************************************************************
import requests
import json


# ----------------------------------------------------------------------------------------------------
# A generic OPTIONS
# ----------------------------------------------------------------------------------------------------
def OPTIONS (server, command, headers={}):
    sessionid = server["sessionid"]
    developerid = server["developerid"]
    geohash = server["geohash"]
    apiurl = server["apiurl"]

    headers.update({"Content-Type":"application/json", "sessionid":sessionid, "developerid":developerid, "location":geohash})
    try:
        req = requests.options(apiurl + "/" + command, headers=headers, verify=False)
        print (req.status_code)
        if ((req.status_code == 200) or (req.status_code == 204)):
            return {"status":req.status_code, "result":json.loads(req.text)}
        else:
            return {"status":req.status_code, "result":{}}
    except:
        return {"status":-500, "result":{}}
# ----------------------------------------------------------------------------------------------------

# ----------------------------------------------------------------------------------------------------
# A generic HEAD
# ----------------------------------------------------------------------------------------------------
def HEAD (server, command, headers={}):
    sessionid = server["sessionid"]
    developerid = server["developerid"]
    geohash = server["geohash"]
    apiurl = server["apiurl"]

    headers.update({"Content-Type":"application/json", "sessionid":sessionid, "developerid":developerid, "location":geohash})
    try:
        req = requests.head(apiurl + "/" + command, headers=headers, verify=False)
        if ((req.status_code == 200) or (req.status_code == 204)):
            return {"status":req.status_code, "result":json.loads(req.text)}
        else:
            return {"status":req.status_code, "result":{}}
    except:
        return {"status":-500, "result":{}}
# ----------------------------------------------------------------------------------------------------

# ----------------------------------------------------------------------------------------------------
# A generic GET
# ----------------------------------------------------------------------------------------------------
def GET (server, command, headers={}):
    sessionid = server["sessionid"]
    developerid = server["developerid"]
    geohash = server["geohash"]
    apiurl = server["apiurl"]

    headers.update({"Content-Type":"application/json", "sessionid":sessionid, "developerid":developerid, "location":geohash})
    try:
        req = requests.get(apiurl + "/" + command, headers=headers, verify=False)
        if ((req.status_code == 200) or (req.status_code == 204)):
            return {"status":req.status_code, "result":json.loads(req.text)}
        else:
            return {"status":req.status_code, "result":{}}
    except:
        return {"status":-500, "result":{}}        
# ----------------------------------------------------------------------------------------------------

# ----------------------------------------------------------------------------------------------------
# A generic POST
# ----------------------------------------------------------------------------------------------------
def POST (server, command, headers={}, body={}):
    sessionid = server["sessionid"]
    developerid = server["developerid"]
    geohash = server["geohash"]
    apiurl = server["apiurl"]

    headers.update({"Content-Type":"application/json", "sessionid":sessionid, "developerid":developerid, "location": geohash})
    try:
        req = requests.post(apiurl + "/" + command, headers=headers, verify=False, json=body)
        if ((req.status_code == 200) or (req.status_code == 204)):
            return {"status":req.status_code, "result":json.loads(req.text)}
        else:
            return {"status":req.status_code, "result":{}}
    except:
        return {"status":-500, "result":{}}
# ----------------------------------------------------------------------------------------------------

# ----------------------------------------------------------------------------------------------------
# A generic PUT
# ----------------------------------------------------------------------------------------------------
def PUT (server, command, headers={}, body={}):
    sessionid = server["sessionid"]
    developerid = server["developerid"]
    geohash = server["geohash"]
    apiurl = server["apiurl"]

    headers.update({"Content-Type":"application/json", "sessionid":sessionid, "developerid":developerid, "location": geohash})
    try:
        req = requests.put(apiurl + "/" + command, headers=headers, verify=False, json=body)
        if ((req.status_code == 200) or (req.status_code == 204)):
            return {"status":req.status_code, "result":json.loads(req.text)}
        else:
            return {"status":req.status_code, "result":{}}
    except:
        return {"status":-500, "result":{}}
# ----------------------------------------------------------------------------------------------------

# ----------------------------------------------------------------------------------------------------
# A generic DELETE
# ----------------------------------------------------------------------------------------------------
def DELETE (server, command, headers={}, body={}):
    sessionid = server["sessionid"]
    developerid = server["developerid"]
    geohash = server["geohash"]
    apiurl = server["apiurl"]

    headers.update({"Content-Type":"application/json", "sessionid":sessionid, "developerid":developerid, "location": geohash})
    try:
        req = requests.delete(apiurl + "/" + command, headers=headers, verify=False, json=body)
        if ((req.status_code == 200) or (req.status_code == 204)):
            return {"status":req.status_code, "result":json.loads(req.text)}
        else:
            return {"status":req.status_code, "result":{}}
    except:
        return {"status":-500, "result":{}}
# ----------------------------------------------------------------------------------------------------