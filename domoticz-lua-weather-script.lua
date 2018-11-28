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

commandArray = {}

if time.sec == 0 then
  -- loop through all the devices
  for deviceName,deviceValue in pairs(otherdevices) do
      if (deviceName=='Aussen') then
          --commandArray['UpdateDevice'] = '8888|0|'..deviceValue
          --print (deviceValue);

          local myTable = deviceValue:split(";")

          if ((tonumber(myTable[1]) < 10) and (tonumber(myTable[1]) >= 0)) then
              commandArray['UpdateDevice'] = '8888|0|  '..myTable[1]
          elseif ((tonumber(myTable[1]) < 0) and (tonumber(myTable[1]) > -10)) then
              commandArray['UpdateDevice'] = '8888|0| '..myTable[1]
          elseif (tonumber(myTable[1]) <= -10) then
              commandArray['UpdateDevice'] = '8888|0|'..myTable[1]
          else
              commandArray['UpdateDevice'] = '8888|0| '..myTable[1]
          end


          --for i = 1, #myTable do
          --   print( myTable[i] ) -- This will give your needed output
          --end
      end
  end
end

return commandArray
