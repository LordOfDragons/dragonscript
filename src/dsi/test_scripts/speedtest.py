import time

def bench():
	vValue = 0
	vRet = 0
	vTime = time.time()
	for i in xrange(150000):
		vValue = 1145677
		vRet = (vValue & 2047) / 8.0
		vRet = ((vValue >> 11) & 2047) / 8.0
		vRet = ((vValue >> 22) & 1023) / 4.0
	vTime = time.time() - vTime
	print 'Finished Test, %f s.' % vTime
	
if __name__ == '__main__':
	bench()
	
# ruby: 2.09 s
# dsi: 0.470000s.
# python: 0.505688 s
# java: 0.0090 s
# scheme compied: 0.27 s