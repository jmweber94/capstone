console.log('Loading function');

// dependencies
var AWS = require('aws-sdk');
var util = require('util');
var config = require('./config.json');

// Get reference to AWS clients
var dynamodb = new AWS.DynamoDB();
var ses = new AWS.SES();
var date = new Date();

function getDevices(items) {
	// Bytesize
	
	var params = {
	    RequestItems: { // map of TableName to list of Key to get from each table
	        TableName: config.DDB_TABLE {
	            Keys: [ // a list of primary key value maps
	                {},
	                // ... more keys to get from this table ...
	            ],
	        },
	        // ... more tables and keys ...
	    },
	};
	BatchGetItemOutcome outcome = dynamodb.batchGetItem(params, function(err, data) {
	    if (err) console.log(err); // an error occurred
	    else console.log(data); // successful response

	});

	Map<String, KeysAndAttributes> unprocessed = null;  

	do {
	    for (String tableName : outcome.getTableItems().keySet()) {
	        System.out.println("Items in table " + tableName);
	        List<Item> items = outcome.getTableItems().get(tableName);
	        for (Item item : items) {
	            System.out.println(item.toJSONPretty());
	        }
	    }
	    // Check for unprocessed keys which could happen if you exceed provisioned
	    // throughput or reach the limit on response size.
	    unprocessed = outcome.getUnprocessedKeys();
	    
	    if (unprocessed.isEmpty()) {
	        System.out.println("No unprocessed keys found");
	    } else {
	        System.out.println("Retrieving the unprocessed keys");
	        outcome = dynamoDB.batchGetItemUnprocessed(unprocessed);
	    }
	} while (!unprocessed.isEmpty());
}

exports.handler = function(event, context) {
	var deviceID = event.deviceID;
	var name = event.name;
	var dob = event.dob;
	var location = event.location;
	var room = event.room;

	getDevices(name, dob, location, room)
};
