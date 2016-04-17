console.log('Loading function');

// dependencies
var AWS = require('aws-sdk');
var crypto = require('crypto');
var util = require('util');
var config = require('./config.json');

// Get reference to AWS clients
var dynamodb = new AWS.DynamoDB();
var ses = new AWS.SES();

function storeDevice(name, dob, location, room) {
	// Bytesize
	dynamodb.putItem({
		TableName: config.DDB_TABLE,
		Item: {
			dob: {
				S: dob
			},
			name: {
				S: name
			},
			location: {
				S: location
			},
			room: {
				S: room
			}
		},
	}, function(err, data) {
		if (err) return fn(err);
		else fn(null, token);
	});
}

exports.handler = function(event, context) {
	var name = event.name;
	var dob = event.dob;
	var location = event.location;
	var room = event.room;
		 
	storeDevice(name, dob, location, room)
};