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
	var len = 128;
	crypto.randomBytes(len, function(err, token) {
		if (err) return fn(err);
		token = token.toString('hex');
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
			ConditionExpression: 'attribute_not_exists (email)'
		}, function(err, data) {
			if (err) return fn(err);
			else fn(null, token);
		});
	});
}

exports.handler = function(event, context) {
	var name = event.name;
	var dob = event.dob;
	var location = event.location;
	var room = event.room;
		 
	storeDevice(name, dob, location, room, function(err, token) {
		if (err) {
			if (err.code == 'ConditionalCheckFailedException') {
				// userId already found
				context.succeed({
					created: false
				});
			} else {
				context.fail('Error in storeDevice: ' + err);
			}
		} 
	});
};