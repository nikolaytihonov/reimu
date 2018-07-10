dofile("base.lua")

function parseSearch(query,num)
	local curl = curl_open()

	local cur = 1
	repeat
		if cur == 1 then
			curl:setOpt(CURLOPT_URL,
				string.format("http://hentai-chan.me/?do=search&subaction=search&story=%s",
					query))
			curl:setOpt(CURLOPT_AUTOREFERER,1)
			curl:setOpt(CURLOPT_REFERER,"http://hentai-chan.me/")
			curl:setOpt(CURLOPT_USERAGENT,"test2.lua")
		else
			curl:setOpt(CURLOPT_URL,"http://hentai-chan.me/index.php?do=search")
			curl:setOpt(CURLOPT_POST,1)
			curl:setOpt(CURLOPT_POSTFIELDS,
				string.format("do=search&subaction=search&search_start=%d&full_search=0&result_from=%d&result_num=40&story=%s",
					cur,(cur*40)+1,query))
		end
		f = io.open(string.format("test2/search%d.html",cur),"wb")
		print(curl:performFile(f))
		f:close()
		cur = cur + 1
	until cur > num
		curl:close()
end

file.mkdir("test2")
parseSearch("Evangelion",5)