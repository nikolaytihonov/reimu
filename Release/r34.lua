--<a href="http://rule34-data-006.paheal.net/_images/27947c643931af4bfeaad2ac6d0dc016/2338109%20-%20Alyx_Vance%20Half-Life%20Half-Life_2%20citizen%20pussydestroyer1.png">Image Only</a>

function download(url)
	dofile("base.lua")

	local s = url:split("/")
	local fname = s[#s-1]..url:match("^.+(%..+)$") --Second

	--os.execute("mkdir "..args[2])
	--local path = args[2].."/"..fname
	local path = args[2].."/"..fname
	file.mkdir(args[2])
	if file.exists(fname) then
		print(string.format("File %s exists!"))
		return
	end

	fc = curl_open()

	fc:setOpt(CURLOPT_URL,url)
	fc:setOpt(CURLOPT_USERAGENT,args[1])

	local res = 1
	local errcnt = 0
	local f = io.open(path,"ab")

	--while not res == 0 and errcnt < 5 do
	--	res = fc:performFile(f)
	--	if not res == 0 then
	--		print(string.format("CURL Error %d",res))
	--		f:flush()
	--	end
	--	errcnt = errcnt + 1
	--end

	repeat
		res = fc:performFile(f)
		if not res == 0 then
			print(string.format("CURL Error %d",res))
			f:flush()
		end
	until res == 0 or errcnt >= 5

	if not res == 0 then
		print(string.format("Download of %s failed due %d",
			path,res))
	end

	f:close()
	fc:close()

	print(path)
end

page = curl_open()
local code = 0


page:setOpt(CURLOPT_URL,string.format("http://rule34.paheal.net/post/list/%s/%d",
	args[2],1))
page:setOpt(CURLOPT_USERAGENT,args[1])

--local text,res = page:perform()
local text = nil
local res = 1
local errnum = 0

while not res == 0 or not text and errnum < 5 do
	text,res = page:perform()
	if not res == 1 then
		print("CURL Error %d",res)
	end
	errnum = errnum + 1
end

prs = tohtml(text)
page:close()

links = {}

for k,v in pairs(prs:toTable()) do
	if v:isTag() then
		if v:tagName() == "a" then
			local href = v:attribute("href")
			if href and prs:contentOf(v) == "Image Only" then
				links[#links+1] = href
				--break
			end
		end
	end
end

print(string.format("Found %d",#links))
performMultiTask(download,links)
page:close()
--for k,v in pairs(links) do print(v) end
