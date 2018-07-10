function string:split( inSplitPattern, outResults )
  if not outResults then
    outResults = { }
  end
  local theStart = 1
  local theSplitStart, theSplitEnd = string.find( self, inSplitPattern, theStart )
  while theSplitStart do
    table.insert( outResults, string.sub( self, theStart, theSplitStart-1 ) )
    theStart = theSplitEnd + 1
    theSplitStart, theSplitEnd = string.find( self, inSplitPattern, theStart )
  end
  table.insert( outResults, string.sub( self, theStart ) )
  return outResults
end

function string:extension()
  return self:match("^.+(%..+)$")
end

function implode(t,sep)
  local str = ""
  for k,v in pairs(t) do
    if #str == 0 then
      str = tostring(v)
    else
      str = ("%s%s%s"):format(str,sep,tostring(v))
    end
  end
  return str
end

function _performCurl(curl,mr)
  local data = nil
  local res = nil
  local code = 0
  local errs = 0

  repeat
    data,res = curl:perform()
    if res == 0 then
      code = curl:getInfo(CURLINFO_HTTP_CODE)
      if code ~= 200 then
        print(("HTTP Error %d"):format(code))
        errs = errs + 1
      end
    else
      print(("CURL Error %d"):format(res))
      errs = errs + 1
    end
    if errs > mr then
      return nil,res,code
    end
  until res == 0 and data ~= nil
  return data,0,code
end

function _performFileCurl(curl,f,mr)
  local res = nil
  local code = 0
  local errs = 0

  repeat
    res = curl:performFile(f)
    if res == 0 then
      code = curl:getInfo(CURLINFO_HTTP_CODE)
      if code ~= 200 then
        print(("HTTP Error %d"):format(code))
        errs = errs + 1
		f:seek("set",0)
		f:flush()
      end
    else
      print(("CURL Error %d"):format(res))
      errs = errs + 1
	  f:seek("set",0)
	  f:flush()
    end
    if errs > mr then
      return nil,res,code
    end
  until res == 0
  return 0,code
end

function last(t) return t[#t] end

function sleep(n)
  os.execute("sleep " .. tonumber(n))
end