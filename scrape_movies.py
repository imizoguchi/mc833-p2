import json
import re
from random import randint

with open('movies2.json') as data_file:    
    data = json.load(data_file)

genre = ["Comedy", "Drama", "Action", "Romance", "Animation", "Horror"]
num = 0
for movie in data['movies']:
	synopsis = re.sub(';', ',', movie['synopsis'])
	synopsis = re.sub('\n', ' ', synopsis)
	print "%s;%s;%s;%s;%s;%d" % (num,movie['title'],movie['release_dates']['theater'],genre[randint(0,5)],synopsis, randint(0,5))
	num = num + 1