alias sgDll {
  return $shortfn($scriptdirSocialGraph.dll)
}

;sito is isores necallint, bus blogai
alias -l sgStartup {
  ;window @SocialGraph
  unset %sgChans
  var %t = $findfile($scriptdirConfigs\,*Config.txt,0,1,sgInitGraph $1-)
}

alias sgShutDown {
  dll -u $sgDll
  unset %sgChans
  .timersg* off
}

#SocialGraph on
on *:start: {
  sgStartup 
}

on *:load: {
  sgStartup 
}

on *:unload: {
  sgShutDown 
}

on *:join:%sgChans: {
  dll $sgDll mAddJoin $chan $nick
}

on *:text:*:%sgChans: {
  ;dll $sgDll mAddMessage $chan $nick $strip($1-)
  var %t = $dllcall($sgDll,return,mAddMessage,$chan $nick $strip($1-))
}

on *:input:%sgChans: {
  ;dll $sgDll mAddMessage $chan $me $strip($1-)
  var %t = $dllcall($sgDll,return,mAddMessage,$chan $me $strip($1-))
}

on *:nick: {
  var %x = $numtok(%sgChans,$chr(44))
  while (%x) {
    var %chan = $gettok(%sgChans,%x,$chr(44))
    if ($newnick ison %chan) {
      dll $sgDll mAddJoin %chan $newnick
    }
    dec %x
  }
}

#SocialGraph end

on *:signal:SocialGraph: {
  ;getNicks genFrame setChan
  ;echo -sg SocialGraph Signal: $1-
  if (genFrame == $1) {
    if ($calc($3 % 100) == 0 && $2 == #linkomanija) {
      msg $2 Reminder: Social Graph is LIVE: http://hddforum.net/PISG/socialgraph/linkomanija.htm (Frame $3 generated) P.s. Graph is updated every few minutes.
    } 
  }
  else if (getNicks == $1) {
    sgCollectNicks $2
  }
  else if (addChan == $1) {
    set %sgChans $addtok(%sgChans,$2,44)
  }
  else if (delChan == $1) {
    set %sgChans $remtok(%sgChans,$2,44)
  }
}

alias sgDisable {
  .disable #SocialGraph 
  sgShutDown 
}

alias sgEnable {
  .enable #SocialGraph
  sgStartup 
}

alias sgCollectNicks {
  if ($me !ison $1) {
    .timersg. $+ $1 1 10 sgCollectNicks $1
    return
  }
  var %x = $nick($1,0)
  while (%x) {
    dll $sgDll mAddJoin $1 $nick($1,%x)
    dec %x
  }
}

;debug tikslais
alias sgDumpFile {
  dll $sgDll mDumpFile $1 $scriptdirManualDump.txt
}

alias sgRender {
  dll $sgDll mMakeImage  $1
}

alias sgUpload {
  dll $sgDll mUpload  $1
}

alias sgAddIgnore {
  dll $sgDll mAddIgnore $1-2
}

alias sgDelIgnore {
  dll $sgDll mDeleteIgnore $1-2
}

alias sgDeleteNode {
  dll $sgDll mDeleteNode $1-2
}

alias sgDeleteUnusedNodes {
  dll $sgDll mDeleteUnusedNodes $1
}

alias sgInitGraph {
  dll $sgDll mInitGraph $1-
}

alias sgDestroyGraph {
  dll $sgDll mDestroyGraph $1
}


alias sgRenderCalc {
  if (!$4) {
    echo -ag Syntax: /sgRenderCalc time1 time2 framesPerDay fps
    return
  }
  var %f = $int($calc($abs($calc($2 - $1)) / 86400 * $3))
  echo -ag ~ Frames: %f </> Duration: $duration($calc(%f / $4),3) </> Real Time: $duration($abs($calc($1 - $2)),2)
}

alias sgRenderVideo {
  var %r = $dllcall($sgDll,return,mRenderVideo,$sfile($$scriptdirConfigs\*config.txt,Select channel config file...,Select))
}
