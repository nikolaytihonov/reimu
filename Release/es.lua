--<span class="thumb"><a id="p4672" href="index.php?page=post&amp;s=view&amp;id=4672"></a>
--<img alt="img" src="http://img.booru.org/es//images/5/4aaad02a2bf945bccd3fd0dc6ce9c6d852a7bd14.jpg" id="image" onclick="Note.toggle();" style="margin-right: 70px;">
--http://es.booru.org/index.php?page=post&s=view&id=4672
--<a href="index.php?page=post&amp;s=list&amp;tags=slavya">slavya</a>
dofile("base.lua")
userAgent = "seVIII"
maxErrors = 15
folder = "es"

task.setThreadCount(10)

function download(id)
	dofile("base.lua")

	local curl = curl_open()
	curl:setOpt(CURLOPT_URL,
		("http://es.booru.org/index.php?page=post&s=view&id=%s"):format(id))
	curl:setOpt(CURLOPT_USERAGENT,task.getGlobal("userAgent"))
	curl:setOpt(CURLOPT_AUTOREFERER,1)

	local src = nil
	local tags = {}
	local maxErrors = tonumber(task.getGlobal("maxErrors"))

	local res = 1
	local data = nil
	local code = 500
	local en = 0
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
	until (res == 0 and code == 200) or en == maxErrors
	if res ~= 0 or code ~= 200 or data == nil then
		print(("Post %s fault"):format(id))
		curl:close()
		return
	end

	local prs = tohtml(data)
	for k,v in pairs(prs:toTable()) do
		if v:tagName() == "img" and src == nil then
			local alt = v:attribute("alt")
			lsrc = v:attribute("src")
			local id = v:attribute("id")
			if lsrc ~= nil and alt == "img"
				and id == "image" then
				if src == nil then src = lsrc end
			end
		elseif v:tagName() == "a" then
			local href = v:attribute("href")
			if href ~= nil then
				local i,_ = href:find("page=post&amp;s=list&amp;tags=")
				if i ~= nil then
					local cont = prs:contentOf(v)
					if cont ~= "Posts" then
						tags[#tags+1] = cont end
				end
			end
		end
	end

	if src == nil then
		print(("Post %s src not found!")
			:format(id))
		curl:close()
		return
	end

	local name = last(src:split("/"))
	local path = task.getGlobal("folder").."/"..name
	if file.exists(path) then
		if file.size(path) == 0 then
			print(("File %s empty, re-downloading")
				:format(path))
			file.remove(path)
		else
			print(("File %s exists"):format(path))
			curl:close()
			return end
	end

	local f = io.open(path,"wb")

	curl:setOpt(CURLOPT_URL,src)
	res = 1
	en = 0
	code = 500

	repeat
		res = curl:performFile(f)
		if res ~= 0 then
			f:close()
			f = io.open(path,"wb")
			print(("CURL Error %d"):format(res))
			en = en + 1
		elseif res == 0 then
			code = curl:getInfo(CURLINFO_HTTP_CODE)
			if code ~= 200 then
				f:close()
				f = io.open(path,"wb")
				print(("HTTP Error %d"):format(code))
				en = en + 1
			end
		else
			if f:seek() <= 1 then
				f:close()
				f = io.open(path,"wb")
				print"Response error"
				en = en + 1
			end
		end
		break
	until (res == 0 and code == 200) or en == maxErrors
	if en == maxErrors then
		print(("Download %s failed due %d %d")
			:format(res,code))
	else print(path) end

	local fold = task.getGlobal("folder")
	task.lockGlobal()
	local index = io.open(fold.."/index.txt","ab")
	index:write(("%s = %s\n")
		:format(name,implode(tags,"+")))
	index:close()
	task.unlockGlobal()

	f:close()
	curl:close()
end

function dumpLinks(code)
	local prs = tohtml(code)
	local ids = {}
	for k,v in pairs(prs:toTable()) do
		if v:tagName() == "a" then
			local id = v:attribute("id")
			local href = v:attribute("href")
			if id ~= nil and id ~= "pi"
				and href ~= nil then
				ids[#ids+1] = id:sub(2)
			end
		end
	end
	return ids
end

function dumpMain(maxPage)
	local curl = curl_open()
	local page = 0
	local links = {}

	repeat
		print(("==== page %d"):format(page))
		curl:setOpt(CURLOPT_URL,
			("http://es.booru.org/index.php?page=post&s=list&pid=%d"):format(page*20))
		curl:setOpt(CURLOPT_USERAGENT,task.getGlobal("userAgent"))
		curl:setOpt(CURLOPT_AUTOREFERER,1)

		local res = 1
		local data = nil
		local code = 500
		local en = 0
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
		until (res == 0 and code == 200) or en > 5
		if res ~= 0 or code ~= 200 or data == nil then
			print(("Page %d fault"):format(page))
		end

		links = dumpLinks(data)
		if links ~= nil then
			performMultiTask(download,links)
		else print"Links not found" end
		page = page + 1
	until page == maxPage
	curl:close()
end

file.mkdir(folder)
dumpMain(tonumber(args[2]))