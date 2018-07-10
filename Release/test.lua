curl = curl_open()

curl:setOpt(CURLOPT_URL,"http://nude-moon.com/8992-online--karfagen-jousou-dorei-yuu-final.html?page=1#top")
curl:setOpt(CURLOPT_REFERER,"http://nude-moon.com/8992-online--karfagen-jousou-dorei-yuu-final.html?page=5")
curl:performFile(io.open("out.html","wb"))