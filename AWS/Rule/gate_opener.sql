SELECT 
topic(3) AS thing_name,
substring(topic(3),0, 10) AS thing_type,
substring(topic(3),11, 15) AS thing_purpose,
substring(topic(3), 16, 20) AS serial_number,
timestamp() AS event_time
FROM 
'$aws/things/+/shadow/update'
WHERE 
state.reported.gate_sensor="BLOCKED" AND
state.reported.gate_actuator="CLOSED"