--view http://hentai-chan.me/online/target
--post http://hentai-chan.me/online/%s#page=%d
--full <img style="max-width:1000px;background-color:white;" src="http://img4.hentai-chan.me/manganew_webp/s/1507470874_shimapan-tachibana-omina-p5-harlem-futaba-hen-/32.webp">
--thumb http://img4.hentai-chan.me/manganew_webp_thumbs/s/1507470874_shimapan-tachibana-omina-p5-harlem-futaba-hen-/01.webp
--thumb <img src="http://img4.hentai-chan.me/manganew_webp_thumbs/s/1507470874_shimapan-tachibana-omina-p5-harlem-futaba-hen-/01.webp" alt="(Стр. 1)" height="140" width="100" class="thumb">

dofile("base.lua")
task.setThreadCount(35)
--task.setDelay(15000)

baseTimeout = 300
maxErrors = 30
domain = "http://hentai-chan.me/"
oldDomain = domain
jsEmpty = [[<script type='text/javascript'> var smartphone_true = 0;</script>]]

index = mutex_open()
mangaIndex = {}


function index_add(link,tags)
	index:lock()
	local f = io.open("index.txt","ab")
	f:write(("%s = "):format(link))
	u8.write(f,tags)
	f:write("\n")
	f:close()
	index:unlock()
end

function dumpDataLinks(manga)
	local curl = curl_open()

	--local url = "http://hentai-chan.me/online/"..manga..".html"
	local url = (domain.."online/%s.html"):format(manga)
	curl:setOpt(CURLOPT_URL,url)
	curl:setOpt(CURLOPT_USERAGENT,"seVII")
	curl:setOpt(CURLOPT_COOKIEFILE,"cookies.txt")
	curl:setOpt(CURLOPT_COOKIEJAR,"cookies.txt")
	curl:setOpt(CURLOPT_REFERER,domain)

	local data = nil
	local code = 500
	local res = 1
	local en = 0

	repeat
		data,res = curl:perform()
		if res == 0 then
			code = curl:getInfo(CURLINFO_HTTP_CODE)
		end
		if res ~= 0 or code ~= 200 then
			print(string.format("CURL Error %d",res))
			en = en + 1
			if en > maxErrors then
				print("Boolshit happends, %d",res)
				return nil
			end
		end
	until res == 0
	curl:close()
	if data == nil or data == jsEmpty then
		local code = curl:getInfo(CURLINFO_HTTP_CODE)
		print(res,code)
		print"New domain, redirecting.."

		_G.oldDomain = domain
		domain = "http://exhentaidono.me/"
		local ret = dumpDataLinks(manga)
		domain = _G.oldDomain

		return ret
	end

	local a = nil
	local new = false
	if domain ~= _G.oldDomain then
		new = true end

	if new then
		_,a = data:find("\"fullimg\": %[")
	else 
		_,a = data:find("\"fullimg\":%[")
	end

	if a == nil then
		local dbg = io.open("err3.html","wb")
		dbg:write(data)
		dbg:close()

		print("Unspeakable error when parsing "..manga)
		return nil
	end

	local b,_ = (data:sub(a+1)):find("]")
	local dat = data:sub(a+1,a+b-2)
	if new then
		return (dat:gsub('\'', '')):split(", ")
	else
		return (dat:gsub('"', '')):split(",")
	end
end

function download(url)
	dofile("base.lua")

	local m = task.getGlobal("manga")
	local curl = curl_open()
	curl:setOpt(CURLOPT_URL,url)

	local lst = last(url:split("/"))
	local ext = lst:extension()
	if ext == nil then
		print(string.format("Malformed url %s",url))
		curl:close()
		return
	end
	local page,_ = lst:find(ext)

	curl:setOpt(CURLOPT_USERAGENT,"seVII")
	curl:setOpt(CURLOPT_COOKIEFILE,"cookies.txt")
	curl:setOpt(CURLOPT_COOKIEJAR,"cookies.txt")
	curl:setOpt(CURLOPT_REFERER,string.format(
		task.getGlobal("domain").."online/%s#page=%d",m,page))
	curl:setOpt(CURLOPT_TIMEOUT,task.getGlobal("baseTimeout"))

	local path = m.."/"..lst
	local f = io.open(path,"wb")
	if f == nil then
		print(("Failed to open %s!"):format(path))
		curl:close()
		return
	end

	local code = 500
	local res = 1
	local en = 0

	repeat
		f:close()
		f = io.open(path,"wb")
		res = curl:performFile(f)
		if res == 0 then
			code = curl:getInfo(CURLINFO_HTTP_CODE)
		end

		if res ~= 0 or (res == 0 and code ~= 200) then
			print("CURL ERROR",res,code)
			en = en + 1
		end
	until code == 200 or en > task.getGlobal("maxErrors")
	if res ~= 0 then
		print(url)
		print"FUCK"
	else print(path) end

	f:close()
	curl:close()
end

function dumpManga(mg)
	manga = mg
	if mg == nil then
		error"SOMETHING WENT VERY VERY WR0000NG"
		return
	end

	if file.exists(manga) then
		print(string.format("Manga %s exists!",manga))
		return
	else file.mkdir(manga) end

	local links = {}
	local fulls = dumpDataLinks(manga)
	if fulls == nil then
		print"dumpDataLinks failed"
		return
	end
	for k,v in pairs(fulls) do
		if v == nil then
			print(("Full %d is nil!"):format(k))
			table.remove(fulls,k)
		end
	end

	--for k,v in pairs(fulls) do
	--	local i,j = v:find(v:extension())
	--	links[k] = v:sub(0,i).."webp"
	--end

	performMultiTask(download,fulls) --jpg
	--performMultiTask(download,links) --webp
end

--<h2><a href="http://hentai-chan.me/manga/22270-marionette-queen-1.0.0.html" >

function dumpTags(data)
	local rs = tohtml(data)
	local tags = {}
	for k,v in pairs(rs:toTable()) do
		if v:isTag() and v:tagName() == "div"
			and v:attribute("class") == "genre" then
			local childs = rs:getChildsOf(v)
			for i,j in pairs(childs) do
				if j:tagName() == "a" then
					tags[#tags+1] = rs:contentOf(j)
				end
			end
		end
	end
end

function getMangaLink(rs,row) --manga_row1
	for k,v in pairs(rs:getChildsOf(row)) do
		if v:isTag() and v:tagName() == "h2" then
			local tag = rs:getChildsOf(v)[1]
			if tag:isTag() and tag:tagName() == "a" then
				local hrefs = tag:attribute("href"):split("/")
				if hrefs[#hrefs-1] == "manga" then
					local link = last(hrefs)
					local i,_ = link:find(link:extension())
					return link:sub(0,i-1)
				end
			end
		end
	end
	return nil
end

function getMangaTags(rs,row) --manga_row3
	local childs = rs:getChildsOf(row)
	local genre = nil
	for k,v in pairs(childs) do
		if v:tagName() == "div" and
			v:attribute("class") == "row3_left" then
			local j = rs:getChildsOf(v)[2]
			if j:tagName() == "div" and
				j:attribute("class") == "item4" then
				genre = rs:getChildsOf(j)[2]
				--print(genre:tagName())
			end
		end
	end

	if genre == nil then return nil end
	local tags = {}

	childs = rs:getChildsOf(genre)
	for k,v in pairs(childs) do
		if v:tagName() == "a" then
			local href = v:attribute("href")
			if href ~= nil then 
				tags[#tags+1] = rs:contentOf(v) end
		end
	end

	return tags
end

function dumpRowContent(prs,content)
	local rows = prs:getChildsOf(content)
	local tags = {}
	local row3skip = false
	local link = nil

	for k,v in pairs(rows) do
		if v:tagName() == "div" then
			local class = v:attribute("class")
			--print(class)
			if class == "manga_row1" then
				link = getMangaLink(prs,v)
			elseif class == "manga_row3" then
				if not row3skip then row3skip = true
				elseif row3skip == true then
					tags = getMangaTags(prs,v)
				end
			end
		end
	end
	return link,implode(tags,"+")
end

function dumpSearch(data)
	local prs = tohtml(data)
	local rows = nil

	local link = nil
	local tags = nil

	for k,v in pairs(prs:toTable()) do
		if v:tagName() == "div" and
			v:attribute("class") == "content_row" then
			link,tags = dumpRowContent(prs,v)

			if link ~= nil then
				dumpManga(link)
				index_add(link,tags)
			end
		end
	end
end

function parseSearch(query,num)
	local curl = curl_open()

	local cur = 1
	repeat
		if cur == 1 then
			curl:setOpt(CURLOPT_URL,
				string.format(domain.."?do=search&subaction=search&story=%s",query))
			curl:setOpt(CURLOPT_AUTOREFERER,1)
			curl:setOpt(CURLOPT_REFERER,("http://%s/"):format(domain))
			curl:setOpt(CURLOPT_USERAGENT,"seVII")
		else
			curl:setOpt(CURLOPT_URL,domain.."index.php?do=search")
			curl:setOpt(CURLOPT_POST,1)
			curl:setOpt(CURLOPT_POSTFIELDS,
				string.format("do=search&subaction=search&search_start=%d&full_search=0&result_from=%d&result_num=40&story=%s",
					cur,(cur*40)+1,query))
		end
		local data = nil
		local res = 1
		local en = 0

		repeat
			data,res = curl:perform()
			if res ~= 0 then
				print(string.format("CURL Error %d",res))
				en = en + 1
				if en > maxErrors then
					print(string.format("Searching page %d failed due %d",cur,res))
					curl:close()
					return
				end
			end
		until (res == 0 and data ~= nil) or en > task.getGlobal("maxErrors")

		dumpSearch(data)
		cur = cur + 1
	until cur > num
		curl:close()
end

function parseTags(tags,num)
	local curl = curl_open()

	curl:setOpt(CURLOPT_USERAGENT,"seVII")
	curl:setOpt(CURLOPT_REFERER,"http://hentai-chan.me/")
	curl:setOpt(CURLOPT_AUTOREFERER,1)

	local en = 0
	local res = 1
	local data = nil
	local code = 500
	local cur = 0

	repeat
		local page = ("offset=%d"):format(cur*20)
		if cur == 0 then page = "" end

		curl:setOpt(CURLOPT_URL,("http://hentai-chan.me/tags/%s?%s"):format(
			tags,page))

			repeat
				data,res = curl:perform()
				if res == 0 then
					code = curl:getInfo(CURLINFO_HTTP_CODE)
					if code ~= 200 then
						print(("HTTP Error %d"):format(code))
					end
				else
					print(("CURL Error %d"):format(res))
					en = en + 1
					if en > 5 then
						curl:close()
						return
					end
				end
			until res == 0 and data ~= nil

		if data == nil then
			print"Server replied illegal page"
			break
		end
		dumpSearch(data)
		cur = cur + 1
	until cur == num
	curl:close()
end

--http://hentai-chan.me/manga/
--http://hentai-chan.me/manga/new?offset=(20*page)
function parseMain(page)
	local curl = curl_open()
	local cur = 0

	curl:setOpt(CURLOPT_REFERER,"http://hentai-chan.me/")
	curl:setOpt(CURLOPT_AUTOREFERER,1)

	repeat
		print(("=================\ncur %d\n"):format(cur))

		if cur == 0 then
			curl:setOpt(CURLOPT_URL,"http://hentai-chan.me/manga/")
		else
			curl:setOpt(CURLOPT_URL,
				("http://hentai-chan.me/manga/new?offset=%d"):format(cur*20))
		end

		local data = nil
		local code = 500
		local res = 1
		local en = 0

		repeat
			data,res = curl:perform()
			if res ~= 0 then
				print(("CURL Error %d"):format(res))
				en = en + 1
			else
				code = curl:getInfo(CURLINFO_HTTP_CODE)
				if code ~= 200 then
					print(("HTTP Error %d"))
					en = en + 1
				end
			end
		until (res == 0 and code == 200) or en == maxErrors

		if res ~= 0 or data == nil then
			print"parseMain failed"
		else dumpSearch(data) end
		cur = cur + 1
	until cur == page
end

if args[2] == "--manga" then
	dumpManga(args[3])
elseif args[2] == "--main" then
	parseMain(tonumber(args[3]))
elseif args[2] == "--search" then
	io.write"Enter query: "
	local query = u8.conv_u16(u8.scan(256))
	parseSearch(query,tonumber(args[3]))
elseif args[2] == "--tags" then
	io.write"Enter tags: "
	local tags = u8.conv_u16(u8.scan(256))
	parseTags(tags,tonumber(args[3]))
else print"Unrecognized mode!" end
