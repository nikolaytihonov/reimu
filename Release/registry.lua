dofile("base.lua")

task.setThreadCount(35)
--task.setDelay(15000)

maxErrors = 30
domain = "http://hentai-chan.me/"
oldDomain = domain
jsEmpty = [[<script type='text/javascript'> var smartphone_true = 0;</script>]]

function index_add(link,tags)
	print(link)

	local f = io.open("registry.txt","ab")
	f:write(("%s = "):format(link))
	u8.write(f,tags)
	f:write("\n")
	f:close()
end

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
					return tag:attribute("href")
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
	return link,implode(tags,",")
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
			index_add(link,tags)
		end
	end
end

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

parseMain(653)