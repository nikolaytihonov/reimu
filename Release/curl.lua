function worker(arg)
	dofile("base.lua")

	curl = curl_open()

	local name = last(arg:split("/"))

	curl:setOpt(CURLOPT_URL,arg)
	curl:setOpt(CURLOPT_USERAGENT,"Lua CURL")

	f = io.open(name..".html","ab")
	print(string.format("Perform %d",
		curl:performFile(f)))
	
	f:close()
	curl:close()
end

local t = {
	"http://lua-users.org/wiki/MetatableEvents",
	"http://lua-users.org/lists/lua-l/2014-04/msg00399.html",
	"http://lua-users.org/wiki/MathLibraryTutorial"
}

performMultiTask(worker,t)