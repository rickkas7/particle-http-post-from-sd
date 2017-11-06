// https://stackoverflow.com/questions/17981677/using-post-data-to-write-to-local-file-with-node-js-and-express
var express = require('express'),
fs = require('fs')
url = require('url');
var app = express();

//app.use('/public', express.static(__dirname + '/public'));  
//app.use(express.static(__dirname + '/public')); 

app.post('/upload', function(request, response) {
	console.log("upload started");
	var body = '';
	filePath = __dirname + '/public/data.txt';
	request.on('data', function(data) {
		console.log("got data " + data.length);
		body += data;
	});

	request.on('end', function (){
		fs.appendFile(filePath, body, function() {
			console.log("file received " + body.length);
			response.end();
		});
	});
});

app.listen(8087);
