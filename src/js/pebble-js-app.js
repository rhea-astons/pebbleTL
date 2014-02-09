var options = {
    appMessage: {
        maxTries: 3,
        retryTimeout: 3000,
        timeout: 100
    },
    http: {
        timeout: 20000
    },
    timer: {
        statusTimer: 5000,
    }
};

var stops = [];
var buses = [];
var appMessageQueue = [];
var locationOptions = { "timeout": 15000, "maximumAge": 60000 };


function sendAppMessageQueue() {
    if (appMessageQueue.length > 0) {
        currentAppMessage = appMessageQueue[0];
        currentAppMessage.numTries = currentAppMessage.numTries || 0;
        currentAppMessage.transactionId = currentAppMessage.transactionId || -1;
        if (currentAppMessage.numTries < options.appMessage.maxTries) {
            Pebble.sendAppMessage(
                currentAppMessage.message,
                function(e) {
                    appMessageQueue.shift();
                    setTimeout(function() {
                        sendAppMessageQueue();
                    }, options.appMessage.timeout);
                }, function(e) {
                    console.log('Failed sending AppMessage for transactionId:' + e.data.transactionId + '. Error: ' + e.data.error.message);
                    appMessageQueue[0].transactionId = e.data.transactionId;
                    appMessageQueue[0].numTries++;
                    setTimeout(function() {
                        sendAppMessageQueue();
                    }, options.appMessage.retryTimeout);
                }
            );
        } else {
            appMessageQueue.shift();
            console.log('Failed sending AppMessage for transactionId:' + currentAppMessage.transactionId + '. Bailing. ' + JSON.stringify(currentAppMessage.message));
        }
    } else {
        console.log('AppMessage queue is empty.');
    }
}

function locationSuccess(pos) {
    console.log('Location found');
    var coords = pos.coords;
    getStops(coords, sendStops);
}

function sendStops() {
    for (var i = 0; i < stops.length && i < 10; i++) {
        appMessageQueue.push({message: {
            index: i,
            id: stops[i].id,
            name: stops[i].name,
            distance: stops[i].distance.toString() + "m"
        }});
    }
    sendAppMessageQueue();
}

function sendBuses() {
    for (var i = 0; i < buses.length; i++) {
        appMessageQueue.push({message: {
            index: i,
            line: buses[i].line,
            timeleft: buses[i].time,
            dest: buses[i].destination
        }});
    }
    sendAppMessageQueue();
}

function locationError(err) {
    console.warn('location error (' + err.code + '): ' + err.message);
    Pebble.sendAppMessage({
        id: 0,
        name: "Location unavailable"
    });
}

function getStops(coords, callback) {
    console.log('Getting stops list');
    var response;
    var i;
    var req = new XMLHttpRequest();
    req.open('GET', 'http://m.t-l.ch/ressources/reseau_markers.php', true);
    req.onload = function(e) {
        if (req.readyState == 4) {
            if (req.status == 200) {
                response = JSON.parse(req.responseText);
                for (i = 0; i < response.length; i++) {
                    response[i]['distance'] = Math.round(getDistanceFromLatLonInKm(coords.latitude, coords.longitude, response[i].latitude, response[i].longitude)*1000);
                    if (response[i].distance < 2000) {
                        stops.push(response[i]);
                        console.log("found stop: " + JSON.stringify(response[i]));
                    }
                }
                stops.sort(function(a,b){
                    return a.distance - b.distance;
                });
                console.log("found " + stops.length + " stops ");
                callback();
            } else {
                console.log('not 200');
            }
        } else {
            console.log('not ready');
        }
    };
    req.send(null);
}

function getBuses(stopId, callback) {
    console.log('Getting buses for ' + stopId);
    var response;
    var i;
    var req = new XMLHttpRequest();
    req.open('GET', 'http://m.t-l.ch/ressources/reseau_horaire.php?id=' + stopId, true);
    //req.open('GET', 'http://m.t-l.ch/ressources/reseau_horaire.php?id=' + '1970329131942227', true);
    req.onload = function(e) {
        if (req.readyState == 4) {
            if (req.status == 200) {
                response = JSON.parse(req.responseText);
                if (response) {
                    console.log(req.responseText);
                    for (i = 0; i < response.length; i++) {
                        if(response[i].time.charAt(0) == '<') {
                            response[i].time = "Now";
                        }
                        buses.push(response[i]);
                    }
                    console.log("found " + buses.length + " buses");
                } else {
                    buses.push({
                        line: "",
                        timeleft: "",
                        dest: "No buses"
                    });
                    console.log("no buses");
                }
                callback();
            } else {
                console.log('not 200');
            }
        } else {
            console.log('not ready');
        }
    };
    req.send(null);
}


function getDistanceFromLatLonInKm(lat1,lon1,lat2,lon2) {
    var R = 6371;
    var dLat = deg2rad(lat2-lat1);
    var dLon = deg2rad(lon2-lon1);
    var a = Math.sin(dLat/2) * Math.sin(dLat/2) +
            Math.cos(deg2rad(lat1)) * Math.cos(deg2rad(lat2)) *
            Math.sin(dLon/2) * Math.sin(dLon/2);
    var c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1-a));
    var d = R * c;
    return d;
}

function deg2rad(deg) {
    return deg * (Math.PI/180);
}

Pebble.addEventListener("ready", function(e) {
    console.log('Try to get location');
    navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions);
});

Pebble.addEventListener("appmessage", function(e) {
    console.log("appmessage received: " + JSON.stringify(e.payload));
    if (e.payload.id) {
        console.log("buslist");
        console.log(e.payload.id);
        getBuses(e.payload.id, sendBuses);
    }
    if (e.payload.index === 0) {
        console.log("stoplist");
        console.log(e.payload.index);
        navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions);
    }
});