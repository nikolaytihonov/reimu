dofile("base.lua")

curl = curl_open()

curl:setOpt(CURLOPT_COOKIEFILE,"")
curl:setOpt(CURLOPT_URL,args[2])
curl:setOpt(CURLOPT_USERAGENT,args[1])

f = io.open("download.html","wb")
print(curl:performFile(f))

f:close()
curl:close()