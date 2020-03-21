# BusRoutes
Give the requested information about buses, stops and optional routes. 
To write requests, that create database and requests to get information u have to use JSON format.
Download JSON parser here https://github.com/nlohmann/json.
Examples of requests:

{

"_comment": "settings for all buses and all stops",

"routing_settings": {
"bus_wait_time": 2,
"bus_velocity": 30
},
"_comment": "create datebase requests",
"base_requests": [

"_comment": "new route request includes name of bus, stops on routes, type of route: roundtrip or not",

{
"type": "Bus",
"name": "297",
"stops": [
"Biryulyovo Zapadnoye",
"Biryulyovo Tovarnaya",
"Universam",
"Biryusinka",
"Apteka",
"Biryulyovo Zapadnoye"
],
"is_roundtrip": true
},
{
"type": "Bus",
"name": "166",
"stops": [
"Tolstopaltsevo"
],
"is_roundtrip": true
},
{
"type": "Bus",
"name": "635",
"stops": [
"Biryulyovo Tovarnaya",
"Universam",
"Biryusinka",
"TETs 26",
"Pokrovskaya",
"Prazhskaya"
],
"is_roundtrip": false
},
{
"type": "Bus",
"name": "828",
"stops": [
"Biryulyovo Zapadnoye",
"TETs 26",
"Biryusinka",
"Universam",
"Pokrovskaya",
"Rossoshanskaya ulitsa"
],
"is_roundtrip": false
},

"_comment": "Stop request includes stop's name, coordinates and distances to other stops",

{
"type": "Stop",
"road_distances": {
"Biryulyovo Tovarnaya": 2600,
"TETs 26": 1100
},
"longitude": 37.6517,
"name": "Biryulyovo Zapadnoye",
"latitude": 55.574371
},
{
"type": "Stop",
"road_distances": {
"Biryusinka": 760,
"Biryulyovo Tovarnaya": 1380,
"Pokrovskaya": 2460
},
"longitude": 37.645687,
"name": "Universam",
"latitude": 55.587655
},
{
"type": "Stop",
"road_distances": {
"Universam": 890
},
"longitude": 37.653656,
"name": "Biryulyovo Tovarnaya",
"latitude": 55.592028
},
{
"type": "Stop",
"road_distances": {
"Apteka": 210,
"TETs 26": 400
},
"longitude": 37.64839,
"name": "Biryusinka",
"latitude": 55.581065
},
{
"type": "Stop",
"road_distances": {
"Biryulyovo Zapadnoye": 1420
},
"longitude": 37.652296,
"name": "Apteka",
"latitude": 55.580023
},
{
"type": "Stop",
"road_distances": {
"Pokrovskaya": 2850
},
"longitude": 37.642258,
"name": "TETs 26",
"latitude": 55.580685
},
{
"type": "Stop",
"road_distances": {
"Rossoshanskaya ulitsa": 3140
},
"longitude": 37.635517,
"name": "Pokrovskaya",
"latitude": 55.603601
},
{
"type": "Stop",
"road_distances": {
"Pokrovskaya": 3210
},
"longitude": 37.605757,
"name": "Rossoshanskaya ulitsa",
"latitude": 55.595579
},
{
"type": "Stop",
"road_distances": {
"Pokrovskaya": 2260
},
"longitude": 37.603938,
"name": "Prazhskaya",
"latitude": 55.611717
},
{
"type": "Stop",
"road_distances": {},
"longitude": 37.20829,
"name": "Tolstopaltsevo",
"latitude": 55.611087
},
{
"type": "Stop",
"road_distances": {},
"longitude": 37.333324,
"name": "Rasskazovka",
"latitude": 55.632761
}
],
"stat_requests": [


"_comment": "Information requests ask for curvature of route, route's length, stops count and unique stops count",

{
"type": "Bus",
"name": "297",
"id": 1
},
{
"type": "Bus",
"name": "635",
"id": 2
},
{
"type": "Bus",
"name": "828",
"id": 3
},
{
"type": "Stop",
"name": "Rasskazovka",
"id": 4
},
{
"type": "Route",
"from": "Biryulyovo Zapadnoye",
"to": "Apteka",
"id": 5
},

"_comment": "Information requests ask for the most optional route bitween two stops, that consists of using buses and stops, 
where we should wait for buses",

{
"type": "Route",
"from": "Biryulyovo Zapadnoye",
"to": "Pokrovskaya",
"id": 6
},
{
"type": "Route",
"from": "Biryulyovo Tovarnaya",
"to": "Pokrovskaya",
"id": 7
},
{
"type": "Route",
"from": "Biryulyovo Tovarnaya",
"to": "Biryulyovo Zapadnoye",
"id": 8
},
{
"type": "Route",
"from": "Biryulyovo Tovarnaya",
"to": "Prazhskaya",
"id": 9
},
{
"type": "Route",
"from": "Apteka",
"to": "Biryulyovo Tovarnaya",
"id": 10
},
{
"type": "Route",
"from": "Biryulyovo Zapadnoye",
"to": "Tolstopaltsevo",
"id": 11
},
{
"type": "Route",
"from":"Tolstopaltsevo" ,
"to":"Biryulyovo Zapadnoye",
"id": 12
},
{
"type": "Route",
"from": "Biryulyovo Zapadnoye",
"to": "Rasskazovka",
"id": 13
},
{
"type": "Route",
"from": "Rasskazovka",
"to": "Biryulyovo Zapadnoye",
"id": 16
},
{
"type": "Route",
"from": "Tolstopaltsevo",
"to": "Tolstopaltsevo",
"id": 14
},
{
"type": "Route",
"from": "Rasskazovka",
"to": "Rasskazovka",
"id": 15
},

"_comment": "Information requests ask for buses, that visit this stop on theirs routes",

{
"type": "Stop",
"name": "Prazhskaya",
"id": 65100610
}
]
}
