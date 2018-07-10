dofile("base.lua")

curl = curl_open()

curl:setOpt(CURLOPT_REFERER,"http://joyreactor.cc/tag/Earth-Chan")

--local ip,res,code = _performCurl(curl,5)
--if res ~= 0 or code ~= 200 then
--	print(("%d %d"):format(res,code))
--else print(ip) end

local f = io.open("test3.html","wb")
_performFileCurl(curl,f,5)
f:close()