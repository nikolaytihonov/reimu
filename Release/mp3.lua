dofile("base.lua")

userAgent = args[1]

function parseArtist(art)
	local curl = curl_open()
	local songs = {}

	curl:setOpt(CURLOPT_URL,("http://mp3party.net/artist/%d"):format(art))
	curl:setOpt(CURLOPT_USERAGENT,task.getGlobal("userAgent"))

	local en = 0
	local res = 1
	local data = nil
	local code = 200

	repeat
		data,res = curl:perform()
		if res ~= 0 then
			print(("CURL Error %d"):format(res))
			en = en + 1
		else
			code = curl:getInfo(CURLINFO_HTTP_CODE)
			if code ~= 200 then
				print(("HTTP Error %d"):format(code))
				en = en + 1
			end
		end
	until res == 0 or en == 5

	if res ~= 0 or data == nil then
		print"parseArtist failed"
		return nil
	end

	local prs = tohtml(data)
	for k,v in pairs(prs:toTable()) do
		if v:tagName() == "div"
			and v:attribute("class") == "name" then
			local link = prs:getChildsOf(v)[2]
			if link ~= nil then
				local href = link:attribute("href")
				if link:tagName() == "a"
					and href:find("/music/") ~= nil then
					songs[#songs+1] = last(href:split("/"))
				end
			end
		end
	end
	return songs
end

function download(id)
	dofile("base.lua")
	local curl = curl_open()

	curl:setOpt(CURLOPT_URL,(
		"http://mp3party.net/music/%d"):format(id))
	curl:setOpt(CURLOPT_USERAGENT,task.getGlobal("userAgent"))
	curl:setOpt(CURLOPT_REFERER,
		("http://mp3party.net/artist/%d"):format(
			task.getGlobal("id")))
	curl:setOpt(CURLOPT_AUTOREFERER,1)

	local en = 0
	local res = 1
	local data = nil
	local code = 500

	repeat
		data,res = curl:perform()
		if res ~= 0 then
			print(("CURL Error %d"):format(res))
			en = en + 1
		else
			code = curl:getInfo(CURLINFO_HTTP_CODE)
			if code ~= 200 then
				print(("HTTP Error %d"):format(code))
				en = en + 1
			end
		end
	until res == 0 or en == 5

	if res ~= 0 or data == nil then
		print(("Donwload %s failed"):format(id))
		curl:close()
		return nil
	end

	local prs = tohtml(data)
	local link = nil
	local name = ("%s.mp3"):format(id)

	for k,v in pairs(prs:toTable()) do
		if v:tagName() == "a"
		and v:attribute("target") == "_blank" then
			local href = v:attribute("href")
			if href:find("mp3party.net/download") ~= nil then
				link = href
				break
			end
		elseif v:tagName() == "div"
			and v:attribute("class") == "breadcrumbs" then
			local childs = prs:getChildsOf(v)
			for i = #childs,1,-1 do
				local node = childs[i]
				if node:tagName() == "span" then
					name = prs:contentOf(node)..".mp3"
					break
				end
			end
		end
	end

	if link == nil then
		print(("Download link for %d not found!"):format(id))
		curl:close()
		return
	end
	curl:setOpt(CURLOPT_URL,link)

	local path = u8.conv_u8(task.getGlobal("folder").."/"..name)
	local mode = u8.conv_u8("wb")
	local f = io.openw(path,mode)
	if f == nil then
		--print(("Failed open %s"):format(path))
		print"Failed open " u8.print(path) print""
		curl:close()
		return nil
	end

	res = 1
	en = 0
	repeat
		res = curl:performFile(f)
		if res ~= 0 then
			f:close()
			f = io.openw(path,mode)
			en = en + 1

			print(("CURL Error %d"):format(res))
		else
			code = curl:getInfo(CURLINFO_HTTP_CODE)
			if code ~= 200 then
				f:close()
				f = io.openw(path,mode)
				en = en + 1

				print(("HTTP Error %d"):format(code))
			end
		end
	until res == 0

	if res ~= 0 or code ~= 200 then
		print"Download " u8.print(path) print""
	else u8.print(path) print"" end
	f:close()
	curl:close()
end

--209987
id = tonumber(args[2])
folder = args[2]
file.mkdir(folder)

local songs = parseArtist(id)
performMultiTask(download,songs)