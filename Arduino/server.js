var http = require('http'),				// require HTTP library
	server = http.createServer(respondToClient);	// create a server with a callback
var hitCount = 0;					// keep track of the number of requests
var recovLevel = 0;
server.listen(84);					// start the server listening
console.log('Server is listening on port 84');

var readline = require('readline');			// create an interface to read lines from the Arduino:
var lineReader = readline.createInterface({
  input: process.stdin,
  output: process.stdout,
  terminal: false
});

var ping = require('ping');
var host = '192.168.1.1';
//var host = 'piotrlech.ddns.net';
var sys = require('sys');
var exec = require('child_process').exec;

var fs = require('fs');

var crypto = require('crypto')
  , beforeHmac = 'I love cupcakes'
  , hash
  , pas = 'password'
  , key = 'key'
  , nonce = 4321277
  , hmac

var SunCalc = require('suncalc');
var times = SunCalc.getTimes(new Date(), 45.1215545, 7.9316955);
var sunset;
var d;

/*sudo npm install cron
Seconds: 0-59
Minutes: 0-59
Hours: 0-23
Day of Month: 1-31
Months: 0-11
Day of Week: 0-6*/
var CronJob = require('cron').CronJob;
new CronJob('5  0 23 * * *', function() { console.log('dlf\n'); }, null, true);
new CronJob('5 30 23 * * *', function() { console.log('dlf\n'); }, null, true);
new CronJob('5  0  0 * * *', function() { console.log('dlf\n'); }, null, true);
//new CronJob('5 ' + times.sunset.getMinutes() + ' ' + times.sunset.getHours() + ' * * *', function() { console.log('pln\n'); }, null, true);
new CronJob('0  0 19 * * *', function() { console.log('plf\n'); }, null, true);
new CronJob('9  9  4 * * *', setSunSet, null, true);
new CronJob('5 * * * * *', chkPing, null, true);

chkPing();
setSunSet();

function respondToClient(request, response) {

	//console.log("request from: ");
	//console.log(request.connection.remoteAddress);
	//console.log("headers:");
	//console.log(request.headers);
	//console.log("URL:");
	//console.log(request.url);

	//beforeHmac = 'The quick brown fox jumps over the lazy dog';
	//beforeHmac = '/' + pas + '/' + request.url + '/';;
	var urlParts = request.url.split('/');
	beforeHmac = '/' + pas + '/' + urlParts[1] + '/' + urlParts[2] + '/';
	hmac = urlParts[3];
	hash = crypto.createHmac('sha256', new Buffer(key, 'utf8')).update(beforeHmac, 'utf8').digest('hex');

	// if you got a POST request, here's the body
	//request.on('data', function(body) {
		//console.log("Body of request:");
	//	console.log(body.toString());
	//});
	// write back to the client:
	response.writeHead(200, {"Content-Type": "text/html"});
	response.write("Hi " + request.connection.remoteAddress);
	response.write(",  hits: " + hitCount);
	//response.write("  request.url: " + request.url);
	//response.write("  beforeHmac: " + beforeHmac);
	//response.write("  hash: " + hash);
	//response.write("  hmac: " + hmac);
	if (hash == hmac) {
		response.write(", hmac ok ");
		if (Number(urlParts[2]) > nonce) {
			response.write(", nonce ok ");
			nonce = Number(urlParts[2]);
			console.log(urlParts[1]);
			//console.log(new Date().toISOString());
			fs.appendFile('/mnt/sda1/arduino/node/moje.txt', (new Date().toISOString()) + '|' + request.connection.remoteAddress + '|' + urlParts[1] + '\n', function (err) {
			});
		}
		else {
			response.write(", nonce nok ");
		}
	}
	else {
		response.write(", hmac nok ");
	}
	response.end();
	// increment the hit counter:
	hitCount++;
}

/* dateFormat (new Date (), "%Y-%m-%d %H:%M:%S", true) returns
   "2012-05-18 05:37:21"  */
function dateFormat (date, fstr, utc) {
  utc = utc ? 'getUTC' : 'get';
  return fstr.replace (/%[YmdHMS]/g, function (m) {
    switch (m) {
    case '%Y': return date[utc + 'FullYear'] (); // no leading zeros required
    case '%m': m = 1 + date[utc + 'Month'] (); break;
    case '%d': m = date[utc + 'Date'] (); break;
    case '%H': m = date[utc + 'Hours'] (); break;
    case '%M': m = date[utc + 'Minutes'] (); break;
    case '%S': m = date[utc + 'Seconds'] (); break;
    default: return m.slice (1); // unknown code, remove %
    }
    // add leading zero if required
    return ('0' + m).slice (-2);
  });
}

function setSunSet() {
	times = SunCalc.getTimes(new Date(), 45.1215545, 7.9316955);
	d = new Date();
	d.setHours(times.sunset.getHours());
	d.setMinutes(times.sunset.getMinutes());
	new CronJob(d, function() {
		/* runs once at the specified date. */
		console.log('pln\n');
		fs.appendFile('/mnt/sda1/arduino/node/moje.txt', (new Date().toISOString()) + '|' + 'sunset\n');
	}, function () {
		/* This function is executed when the job stops */
	},
	true /* Start the job right now */
	);
}

function chkPing() {
  ping.sys.probe(host, function(isAlive){
    var msg = isAlive ? 'host ' + host + ' is alive' : 'host ' + host + ' is dead';
    //console.log(msg);
    if (isAlive) {
      recovLevel = 0;
    }
    else if (recovLevel < 15) {
      fs.appendFile('/mnt/sda1/arduino/node/moje.txt', (new Date().toISOString()) + '|' + recovLevel + '|'  + 'wlan0 down\n');
      child = exec("/root/wlan_rst", function (error, stdout, stderr) {  });
      recovLevel++;
    }
    else {
      fs.appendFile('/mnt/sda1/arduino/node/moje.txt', (new Date().toISOString()) + '|' + recovLevel + '|'  + 'reboot\n');
      child = exec("reboot", function (error, stdout, stderr) {  });
      recovLevel++;
    }
  });
}
