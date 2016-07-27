corridor(c1).
corridor(c2).
corridor(c3).
corridor(c4).
corridor(c5).
corridor(c6).
corridor(c7).
corridor(c8).
corridor(c9).

open(open1).
open(open2).
open(open3).

office(o1).
office(o2).
office(o3).
office(o4).
office(o5).
office(o6).
office(o7).
office(o8).
office(o9).
office(o10).
office(o11).
office(o12).
office(o13).
office(o14).

waypoint(X) :- corridor(X).
waypoint(X) :- open(X).
waypoint(X) :- office(X).

connection(X,Y) :- connection(Y,X).

connection(c1, open1).
connection(c1, c2).
connection(c1, c9).
connection(c2, c3).
connection(c3, o4).
connection(c3, o14).
connection(c3, o13).
connection(c3, o5).
connection(c3, c4).
connection(c4, o12).
connection(c4, o6).
connection(c4, o11).
connection(c4, o7).
connection(c4, c5).
connection(c5, o10).
connection(c5, o9).
connection(c5, c6).
connection(c6, c7).
connection(c7, o8).
connection(c7, open3).
connection(open3, c8).
connection(c8, o3).
connection(c8, open2).
connection(c8, c9).
connection(c9, o2).
connection(c9, o1).
connection(open2, open1).

init(at(mailbot,open3)).
