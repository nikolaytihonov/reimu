dofile("base.lua")

maxThreads = 20
maxErrors = 10
userAgent = "WARP v1.0"

--global dump data
dumpFolder = nil

task.setThreadCount(maxThreads)

function download(url)
	dofile("base.lua")
	local folder = task.getGlobal("dumpFolder")
	local mr = task.getGlobal("maxErrors")
	local src = nil

	local curl = curl_open()
	curl:setOpt(CURLOPT_URL,"http://knowyourmeme.com"..url)
	curl:setOpt(CURLOPT_USERAGENT,"WARP v1.0")
	local data,res,code = _performCurl(curl,mr)
	if data == nil then
		print(("Scan of %s failed due <%d,%d>"):format(url,res,code))
		curl:close()
		return
	end

	local rs = tohtml(data)
	for k,v in pairs(rs:toTable()) do
		if v:tagName() == "img" and v:attribute("class") 
			== "colorbox_photo" then
			src = v:attribute("src")
			break
		end
	end

	if src == nil then
		print("Pic URL not found!")
		curl:close()
		return
	end

	curl:setOpt(CURLOPT_URL,src)
	local last = string.gsub(last(src:split("/")),"_large","")
	local path = ("%s/%s"):format(folder,last)
	if file.exists(path) then
		print(("%s exists!"):format(path))
		curl:close()
		return
	end
	local f = io.open(path,"wb")
	local res,code = _performFileCurl(curl,f,mr)
	if res ~= 0 or code ~= 200 then
		print("Download of %s failed due <%d,%d>",path,res,code)
	else print(path) end
	f:close()
end

function dump(name)
	local curl = curl_open()

	local res = 0
	local data = nil
	local page = 1

	local finish = false

	file.mkdir(name)
	curl:setOpt(CURLOPT_USERAGENT,userAgent)
	repeat
		print(("== page %d"):format(page))
		curl:setOpt(CURLOPT_URL,("http://knowyourmeme.com/memes/%s/photos/page/%d")
		:format(name,page))
		data,res,_ = _performCurl(curl,maxErrors)
		local photos = {}
		if data ~= nil then
			local rs = tohtml(data)
			for k,v in pairs(rs:toTable()) do
				if v:tagName() == "a" and 
						v:attribute("rel") == "photo_gallery" then
					table.insert(photos,v:attribute("data-colorbox-url"))
				elseif v:tagName() == "h3" and v:attribute
					("class")  == "closed" then
					finish = true
					break
				end
			end
		end

		if next(photos) == nil then
			finish = true
			break
		end

		dumpFolder = ("%s/%d"):format(name,page)
		file.mkdir(dumpFolder)
		performMultiTask(download,photos)
		page = page + 1
	until (finish == true) or (res ~= 0 or data == nil)
	if finish == true then
		print("wassup it's a finish!")
	end
end

dump(args[2])