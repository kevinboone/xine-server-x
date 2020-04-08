var API_BASE = "/api/"

var transport_status_interval = 5000;
var scanner_status_interval = 2000;

var timeout_msg = 0;

const TRANSPORT_STOPPED = 0;
const TRANSPORT_PLAYING = 1;
const TRANSPORT_PAUSED = 2;
const TRANSPORT_BUFFERING = 3;

function clear_message ()
  {
  document.getElementById ("messagecell").innerHTML = "";
  timeout_msg = 0;
  }

function cmd_next ()
  {
  var apiFn = API_BASE + "next?dummy";
  make_fn_request (apiFn, response_callback_gen_status);
  }

function cmd_clear ()
  {
  var apiFn = API_BASE + "clear?dummy";
  make_fn_request (apiFn, response_callback_gen_status);
  document.getElementById ("playlist").innerHTML = "Playlist is empty";
  }

function cmd_play_station (list, name)
  {
  var apiFn = API_BASE + "play_station?list=" + encodeURIComponent (list)
    + "&" + "name=" + encodeURIComponent (name);
  make_fn_request (apiFn, response_callback_gen_status);
  }

function cmd_play_dir (path)
  {
  var apiFn = API_BASE + "play_dir?dir=" + encodeURIComponent (path); 
  make_fn_request (apiFn, response_callback_gen_status);
  }

function cmd_add_dir (path)
  {
  var apiFn = API_BASE + "add_dir?dir=" + encodeURIComponent (path); 
  make_fn_request (apiFn, response_callback_gen_status);
  }

function cmd_add_file (path)
  {
  var apiFn = API_BASE + "add_file?file=" + encodeURIComponent (path); 
  make_fn_request (apiFn, response_callback_gen_status);
  }

function cmd_play_file (path)
  {
  var apiFn = API_BASE + "play_file?file=" + encodeURIComponent (path); 
  make_fn_request (apiFn, response_callback_gen_status);
  }

function cmd_play_album (album)
  {
  var apiFn = API_BASE + "play_album?album=" + encodeURIComponent (album); 
  make_fn_request (apiFn, response_callback_gen_status);
  }

function cmd_play_index (index)
  {
  var apiFn = API_BASE + "play_index?index=" + encodeURIComponent (index); 
  make_fn_request (apiFn, response_callback_gen_status);
  }

function cmd_add_matching (args)
  {
  var apiFn = API_BASE + "add_matching?" + args; 
  make_fn_request (apiFn, response_callback_gen_status);
  }

function cmd_play_matching (args)
  {
  var apiFn = API_BASE + "play_matching?" + args; 
  make_fn_request (apiFn, response_callback_gen_status);
  }

function cmd_prev ()
  {
  var apiFn = API_BASE + "prev?dummy";
  make_fn_request (apiFn, response_callback_gen_status);
  }

function cmd_stop ()
  {
  var apiFn = API_BASE + "stop?dummy";
  make_fn_request (apiFn, response_callback_gen_status);
  }

function cmd_shutdown ()
  {
  var apiFn = API_BASE + "shutdown?dummy";
  make_fn_request (apiFn, response_callback_gen_status);
  }

function cmd_play ()
  {
  var apiFn = API_BASE + "play?dummy";
  make_fn_request (apiFn, response_callback_gen_status);
  }

function cmd_full_scan ()
  {
  var apiFn = API_BASE + "full_scan?dummy";
  make_fn_request (apiFn, response_callback_gen_status);
  }

function cmd_quick_scan ()
  {
  var apiFn = API_BASE + "quick_scan?dummy";
  make_fn_request (apiFn, response_callback_gen_status);
  }

function cmd_pause ()
  {
  var apiFn = API_BASE + "pause?dummy";
  make_fn_request (apiFn, response_callback_gen_status);
  }

function do_http_request_complete (callback, http_request)
  {
  if (http_request.readyState == 4)
    {
    //if (http_request.status == 200)
      {
      set_message ("");
      callback (http_request.responseText);
      }
    }
  }

// Make an HTTP request on the specified uri, and call callback with
//  the results when complete
function make_request (uri, callback)
  {
  var http_request = false;
  if (window.XMLHttpRequest)
    { // Mozilla, Safari, ...              
    http_request = new XMLHttpRequest();
    if (http_request.overrideMimeType)
      {
      http_request.overrideMimeType('text/plain');
      }
    }
  else if (window.ActiveXObject)
    { // IE              
    try
      {
      http_request = new ActiveXObject("Msxml2.XMLHTTP");
      }
    catch (e)
      {
      try
        {
        http_request = new ActiveXObject("Microsoft.XMLHTTP");
        }
      catch (e)
        {}
      }
    }
if (!http_request)
    {
    alert('Giving up :( Cannot create an XMLHTTP instance');
    return false;
    }
  http_request.onreadystatechange = function()
    {
    do_http_request_complete (callback, http_request);
    };
  http_request.open('GET', uri, true);
  http_request.timeout = 10000; // Got to have _some_ value
  //http_request.timeout = show_server_timeout();
  http_request.send (null);
  }

function make_fn_request (fn, callback)
  {
  self_uri = parse_uri (window.location.href);

  // The 'random' param is added to work around a stupid caching
  //  bug in IE
  fn_uri = "http://" + self_uri.host  + ":" + self_uri.port +
    fn + "&random=" + Math.random();

  make_request (fn_uri, callback);
  }

// Called on loading main page
function onload_transport ()
  {
  var slider = document.getElementById("volumeslider");
  slider.oninput = function() 
    { set_volume (this.value); }
  setInterval (transport_status_tick, transport_status_interval);
  }

// Called on loading scanner.html, to initialize the scanner
//   status time. 
function onload_scanner ()
  {
  setInterval (scanner_status_tick, scanner_status_interval);
  }

//parse_uri
//Parse a uri into host, port, etc. Result are obatined in a structure
function parse_uri (str) {
        var     o   = parse_uri.options,
                m   = o.parser[o.strictMode ? "strict" : "loose"].exec(str),
                uri = {},
                i   = 14;

        while (i--) uri[o.key[i]] = m[i] || "";

        uri[o.q.name] = {};
        uri[o.key[12]].replace(o.q.parser, function ($0, $1, $2) {
                if ($1) uri[o.q.name][$1] = $2;
        });

        return uri;
};
parse_uri.options = {
        strictMode: false,
        key: ["source","protocol","authority","userInfo","user","password","host","port","relative","path","directory","file","query","anchor"],
        q:   {
                name:   "queryKey",
                parser: /(?:^|&)([^&=]*)=?([^&]*)/g
        },
        parser: {
                strict: /^(?:([^:\/?#]+):)?(?:\/\/((?:(([^:@]*)(?::([^:@]*))?)?@)?([^:\/?#]*)(?::(\d*))?))?((((?:[^?#\/]*\/)*)([^?#]*))(?:\?([^#]*))?(?:#(.*))?)/,
                loose:  /^(?:(?![^:@]+:[^:@\/]*@)([^:\/?#.]+):)?(?:\/\/)?((?:(([^:@]*)(?::([^:@]*))?)?@)?([^:\/?#]*)(?::(\d*))?)(((\/(?:[^?#](?![^?#\/]*\.[^?#\/.]+(?:[?#]|$)))*\/?)?([^?#\/]*))(?:\?([^#]*))?(?:#(.*))?)/
        }
};

function refresh_playback_status()
  {
  var apiFn = API_BASE + "status?dummy";
  make_fn_request (apiFn, response_callback_refresh_playback_status);
  }

function refresh_scanner_status()
  {
  var apiFn = API_BASE + "scanner_status?dummy";
  make_fn_request (apiFn, response_callback_refresh_scanner_status);
  }

// A general callback to be attached to server commands that generate
//  no specific response except a status message (play, 
//  add_to_playlist, etc)
function response_callback_gen_status (response_text)
  {
  var obj = eval ('(' + response_text + ')');
  set_message (obj.message);
  }

// A general callback to be attached to server commands that can not 
//   usefully display anything, such as adjusting the volume slider 
function response_callback_null (response_text)
  {
  }

// response_callback_refresh_playback_status 
// Called in response to the timed status update tick
function response_callback_refresh_playback_status (response_text)
  {
  var obj = eval ('(' + response_text + ')'); 

  if (obj.status != "0")
    {
    // Should we take some action here? It should never happen
    return;
    }

  if (obj.transport_status == TRANSPORT_STOPPED)
    {
    document.getElementById ("transportstatusspan").innerHTML = "stopped";

    document.getElementById ("streamposspan").innerHTML = ("00:00"); 
    document.getElementById ("streamlenspan").innerHTML = ("00:00"); 
    }
  else
    {
    if (obj.transport_status == TRANSPORT_PLAYING)
      document.getElementById ("transportstatusspan").innerHTML = "playing"
    else if (obj.transport_status == TRANSPORT_PAUSED)
      document.getElementById ("transportstatusspan").innerHTML = "paused"
    else if (obj.transport_status == TRANSPORT_BUFFERING)
      document.getElementById ("transportstatusspan").innerHTML = "buffering"

    if (obj.pos > 0)
      document.getElementById ("streamposspan").innerHTML = 
         sec_to_minsec (obj.pos);
    else
      document.getElementById ("streamposspan").innerHTML = ("00:00"); 

    if (obj.len > 0)
      document.getElementById ("streamlenspan").innerHTML = 
        sec_to_minsec (obj.len); 
    else
      document.getElementById ("streamlenspan").innerHTML = ("00:00"); 
    }

  document.getElementById ("titlespan").innerHTML = obj.title; 
  document.getElementById ("albumspan").innerHTML = obj.album; 
  if (obj.playlist_index >= 0)
    document.getElementById ("playlistindexspan").innerHTML 
      = obj.playlist_index + 1; 
  else
    document.getElementById ("playlistindexspan").innerHTML = "?";
  document.getElementById ("playlistlengthspan").innerHTML 
    = obj.playlist_length; 
  }

// response_callback_refresh_playback_status 
// Called in response to the timed scanner status update tick
function response_callback_refresh_scanner_status (response_text)
  {
  var obj = eval ('(' + response_text + ')'); 

  if (obj.status != "0")
    {
    // Should we take some action here? It should never happen
    return;
    }

  if (obj.running == 0)
    document.getElementById ("scannerprogresscell").innerHTML 
      = "File scanner is not running at present"; 
  else
    {
    var msg = "Files scanned: " + obj.scanned 
       + ", entries added to index: " + obj.added
       + ", index entries modified: " + obj.modified
       + ", index entries deleted: " + obj.deleted
       + ", cover images extracted: " + obj.extracted;
    document.getElementById ("scannerprogresscell").innerHTML = msg;
    }
  }


// Called on the scanner page, to update scanner status from server
function scanner_status_tick()
  {
  refresh_scanner_status();
  }


function sec_to_minsec (totalsec)
  {
  if (totalsec < 0) totalsec = 0; // Work around Xine bug
  var min = Math.floor (totalsec / 60);
  var sec = totalsec - min * 60;
  var smin = "" + min;
  if (min < 10) smin = "0" + smin;
  var ssec = "" + sec;
  if (sec < 10) ssec = "0" + ssec;
  return "" + smin + ":" + ssec; 
  }

function set_message (msg)
  {
  document.getElementById ("messagecell").innerHTML = msg;
  if (timeout_msg != 0) 
    clearTimeout (timeout_msg);
  timeout_msg = setTimeout (clear_message, 3000);
  }

function set_volume (volume)
  {
  var apiFn = API_BASE + "set_volume?volume=" + volume;
  make_fn_request (apiFn, response_callback_null);
  }

function transport_status_tick()
  {
  refresh_playback_status();
  }


