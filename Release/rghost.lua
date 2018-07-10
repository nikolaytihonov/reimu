--http://rgho.st/sitemap_recent.xml
--<a href="http://rgho.st/download/67x6DFC5R/b81e972d454b501f0eb3f7ae0fb62f607ba899f4/%D0%9E%D1%81%D0%BD%D0%BE%D0%B2%D0%BD%D1%8B%D0%B5%20%20%D0%BC%D0%BE%D0%BB%D0%B5%D0%BA%D1%83%D0%BB%D1%8F%D1%80%D0%BD%D0%BE-%20%D0%BA%D0%B8%D0%BD%D0%B5%D1%82%D0%B8%D1%87%D0%B5%D1%81%D0%BA%D0%BE%D0%B9.doc" class="btn btn-primary btn-download m-t-10 m-b-5" data-update-url="/files/67x6DFC5R/link" id="download-btn" rel="nofollow" title="Скачать Основные  молекулярно- кинетической.doc">
--<i class="fa fa-download"></i> Скачать</a>

--Tag name "a"
--href, id = "download-btn", rel = "nofollow"

userAgent = "Mozilla/5.0 (Windows NT 10.0; WOW64; rv:40.0) Gecko/20100101 Firefox/40.0"

function parseMain()
	local curl = curl_open()

	curl:setOpt(CURLOPT_URL,"http://rgho.st/sitemap_recent.xml")
	curl:setOpt(CURLOPT_USERAGENT,userAgent)
	curl:setOpt(CURLOPT_COOKIEFILE,"cookies.txt")
	curl:setOpt(CURLOPT_COOKIEJAR,"cookies.txt")
	curl:setOpt(CURLOPT_FOLLOWLOCATION,1)
	curl:setOpt(CURLOPT_AUTOREFERER,1)

	local en = 0
	local data = nil
	local res = 1
	while not (res == 0) do
		print(res)
		data,res = curl:perform()
		if not (res == 0) then
			print(string.format("CURL Error %d",res))
			en = en + 1
			if en > 5 then
				print("Request failed!")
				curl:close()
				return
			end
		end
	end

	local links = {}
	local prs = tohtml(data)
	for k,v in pairs(prs:toTable()) do
		if v:isTag() and v:tagName() == "loc" then
			--print(prs:contentOf(v))
			links[#links+1] = prs:contentOf(v)
		end
	end
	curl:close()
	return links
end

function download(url)
	dofile("base.lua")
	local targets = {
		".jpg", ".png", ".gif", ".bmp",
		".dll", ".cpp", ".c", ".h",
		".rar", ".zip"
		--".dll", ".exe", ".rar"
	}

	curl = curl_open()

	curl:setOpt(CURLOPT_URL,url)
	curl:setOpt(CURLOPT_USERAGENT,task.getGlobal("userAgent"))
	curl:setOpt(CURLOPT_COOKIEFILE,"cookies.txt")
	curl:setOpt(CURLOPT_COOKIEJAR,"cookies.txt")
	curl:setOpt(CURLOPT_FOLLOWLOCATION,1)
	curl:setOpt(CURLOPT_AUTOREFERER,1)

	local en = 0
	local ten = 0
	local data = nil
	local link = nil

	res = 1

	while not (res == 0) do
		data,res = curl:perform()
		if not (res == 0) then
			print(string.format("CURL Error %d",res))
			en = en + 1
		elseif en > 5 then
			print(string.format("Download of %s failed!",url))
			curl:close()
		end
	end

	local prs = tohtml(data)

	for k,v in pairs(prs:toTable()) do
		if v:isTag() and v:tagName() == "a" then
			local href = v:attribute("href")
			local rel = v:attribute("rel")
			local id = v:attribute("id")
			if id == "download-btn" then --and rel == "nofollow" then
				link = href
				break
			end
		end
	end

	if link == nil then
		print("Not downloadable!")
		curl:close()
		return
	end

	local ext = last(link:split("/")):extension()
	local dwnld = false
	for k,v in pairs(targets) do
		if ext == v then dwnld = true end
	end
	if not dwnld then
		print(string.format("%s not in our interests!",url))
		--curl:close()
		--return
	end

	local path = task.getGlobal("dirPath").."/"..last(link:split("/"))
	if not (string.find(path,"sa-mp") == nil) then
		print("SKipping samp boolshit")
		curl:close()
		return
	end
	curl:setOpt(CURLOPT_URL,link)

	f = io.open(path,"ab")
	local res = 0
	en = 0
	res = 1
	while not (res == 0) do
		res = curl:performFile(f)
		if not (res == 0) then
			print(string.format("CURL Error %d",res))
			f:flush()
			en = en + 1
			if en > 5 then
				print(string.format("Download of %s failed due %d",path,res))
				f:close()
				curl:close()
				return
			end
		end
	end
	
	print(path)
	f:close()
	curl:close()
end

dirPath = os.date("rghost_%d_%m_%Y__%H_%M_%S")
file.mkdir(dirPath)
dofile("base.lua")

links = parseMain()
--for k,v in pairs(links) do
--	download(v)
	--sleep(2)
--end
--task.setThreadCount(15)
task.setDelay(500)
performMultiTask(download,links)