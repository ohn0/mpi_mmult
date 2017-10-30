f = open('massiveIdentity', 'w+')

for i in range(500):
	for j in range(500):
		if i == j:
			f.write(str(1) + ' ')
		else:
			f.write(str(0) + ' ')
	f.write('\n')
f.close()
