require "time"

def bench	
	vValue = 0
	vRet = 0
	vTime = Time.now
	150000.times do
		vValue = 1145677
		vRet = (vValue & 2047) / 8.0
		vRet = ((vValue >> 11) & 2047) / 8.0
		vRet = ((vValue >> 22) & 1023) / 4.0	
	end
	vTime = Time.now - vTime
	puts "Finished Test, %fs." % vTime
end

bench