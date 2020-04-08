# xine-server-x (XSX)

*Please note that this project is a work in progress*. Although it is
basically functional, and has been tested on a number of different
Linux installations, it still lacks important functionality, and may
not be entirely reliable. See the file TODO.md for more information.

## What is this?

`xine-server-x`, also known as XSX, is an HTTP-enabled front end to the
`xine-server` distributed audio player, for Linux. 
`xine-server` is available here:

https://github.com/kevinboone/xine-server

Together these components make up a browser-controllable audio player,
for both local files and Internet radio streams. Both `xine-server` and
`xine-server-x` are intended to run on embedded Linux appliances, 
typically based on Raspberry Pi 3/4. 

XSX communicates with `xine-server` using a custom client-server
protocol on (by default) port 30001. In principle, XSX and
`xine-server` could be on separate hosts, but this is only 
practicable if the audio files to be played are on some shared
filesystem. Even then, this isn't really the way these two components
are designed to be used. 

XSX is one of a number of clients of `xine-server` -- another is the
GTK-based Internet radio stream player `gxsradio`:

https://github.com/kevinboone/gxsradio

## Preparation 

XSX relies on a working installation of `xine-server`, usually on the
same host. In general, `xine-server` is harder to set up, because it
requires configuration for specific audio hardware. The `xine-server`
bundle comes with a command-line player called `xine-client`, and 
it should be tested using this client before using anything more
sophisticated. In any event, there's little point testing XSX unless
it's clear that `xine-server` is installed and working properly.

XSX requires a source of audio -- either local files, or radio streams,
or both. All widely-used (at the time of writing) audio file formats 
are supported. Radio streams are specified in files with the same
format as that used by `gxsradio`. 

XSX requires a configuration file, or an extensive list of
command-line arguments -- see below for more information.

## Building 

The only non-obvious dependency that XSX has is GNU `libmicrohttpd`. 
You can get this using `dnf install libmicrohttpd-devel` or
`apt-get install libmicrohttpd-dev`, or whatever is appropriate for your
system. `libmicrohttpd` has a number of dependencies of its own which,
with luck, the package manager will sort out.

Then it's just the usual

    $ make
    $ sudo make install

Please note that XSX is designed to be build using the GNU C compiler;
it uses a number of compiler-specific features.

The binary `xine-server-x` embeds all its HTML content, images, etc.,
to make it easier to install on embedded systems.

## Configuration

XSX needs a certain amount of configuration to operate correctly.
This can be specified using the command line options, or an
RC file -- both methods are described below. 

Of the various configuration
settings, `root` (the audio base directory) is essential, and has
no default. If this setting is not supplied, XSX will not start.
It is nearly always necessary to specify an index file as well --
XSX will start without one, but its functionality will be limited.
The first time that XSX is run, the index file will probably not 
exist; it will be created on the first full file scan (see the
Admin page in the web interface).

## Configuration file

The configuration is read from `/etc/xine-server-x.rc`, and 
`$HOME/.xine-server-x.rc`, with the latter taking priority. The settings
that can be made in the configuration file are identical to the long names
of the command-line options (see below). 

There is a sample configuration file in the `samples/` directory.

## Command line options

`-d,--debug`

Do not detach from the controlling terminal -- run in foreground.
Logging is to `stdout` and all log levels are allowed. This mode of
operation is not the default -- the default is to run as a daemon 
and to log to the system log. 

`-g,--gxsradio={directory}`

The directory in which radio station list files are stored. These are
in `gxsradio` format. The default is `/usr/shared/gxsradio`.

`-i,--index={filename}`

The name of the file used to store the index (database) of audio
files. In nearly all usage scenarios this option needs to be specified,
and the program will emit a warning on start-up if it is not.
The file need not initially exist -- it is created when scanning
audio files. 

This file needs to be in a writeable location -- usually only an
issue on embedded systems. It can be in the audio root directory,
but there is no requirement for it to be.

The index is in sqlite3 format, and can be read (and potentially
modified) by other tools if required.

`l,--log-level={0..4}`

Sets the system log level. In normal (background, daemon) operation,
only log levels 0-2 are captured. In debug (foreground) mode, all log
levels 0-4 can be used. However, high log levels will probably be
meaningful only if examined alongside the program's source code. 

`-p,--port={number}`

The main port number for `xine-server-x`, that is used by web browsers and
REST API clients. The default is 30000.

`-q,--quickscan`

Perform a quick scan of files in the audio root directory, and update the
index. Only files that have been added since the last scan, or changed since
they were added to the index, are scanned. This operation can be invoked
from the command line, or using the web interface. Once scanning has finished,
the program exists. In this mode, other command-line options except
`--index` and `--root` are ignored.

`-r,--root={directory}`

The root directory for local audio files. If audio files are in many
disparate directories, it is fine to create a new directory and populate
it with symbolic links to the real file locations. The scanning process
will descend subdirectories to arbitrary depth.

`-s,--scan`

Perform a full scan of files in the audio root directory, and build the index.
This operation can be invoked from the command line, or using the web
interface. Once scanning has finished, the program exists.  In this mode, other
command-line options except `--index` and `--root` are ignored.

The full scan works on a temporary index file, which has the same
path as the main index with `temp` added. When the scan is complete,
the temporary file overwrites the main index, if it exists. This process
allows the original index, if present, to be used while a full scan
is in progress -- in large audio collections a full scan can take
many minutes.

The scan also extracts cover art images --if present -- from audio files,
if there are none already in the same directory.


`--xsport={number}`

The port number of the `xine-server` instance that this server will
control. The default is 30001.

`--xshost={hostname}` 

The hostname or IP number of the `xine-server` instance that this server
will control. The default is `localhost`, and there are few applications
where it will need to be anything else.

`--xslaunch={command}`

Command to launch `xine-server`, if request. If this option is not 
specified, `xine-server-x` assumes that `xine-server` is already running.
If `xine-server-x` starts `xine-server`, then it will attempt to stop
it on exit. This is the case whether `xine-server-x` is shut down using
an administrative action, or it traps a signal.

The command specified can have arguments of its own; that is, it
can contain spaces. However, in such a case, the whole command will
probably need to be enclosed in quotes to prevent the shell splitting
it up.

It will not usually be possible to start multiple instances of 
`xine-server` on the same port -- after the first instance, later
start attempts will be unable to open the port. It's possible to
configure `xine-server-x` to start a private `xine-server` instance 
on a pivate port, like this:

    $ xine-server --xsport 1234 --xslaunch "xine-server --port 1234"

However, the audio hardware may, or may not, allow multiple processes,
depending on what Xine audio driver is in use.

`--xslaunch` is, of course, unhelpful if `xine-server-x` and `xine-server`
have to run under different user accounts and _a fortiori_ if they
run on different hosts.

Because there may be circumstances in which `xine-server-x` starts 
with `xine-server` unexpectedly still running, it is not considered an
error if `xine-server` cannot be started.

*Important*: whatever command is specified for `--xslaunch`, it must
return quickly. It will not be put in the background. So there's no
point using, for example, `xine-server --debug`, which remains in 
the foreground. Of course, you can still start `xine-server` this way
separately, and not allow `xine-server-x` to try to start it. 

## Logging

Other than when run in debug mode (``--debug``) logging is to
the system log daemon via the `syslog()` system call. Only
log messages of severity 0-2 are logged this way. In debug mode,
logging is to `stdout`, and `--log-level` can be used to set
verbose logging levels 3-4.

What happens to log messages in normal (non-debug) mode depends on
how system logging is configured.

Although it should be obvious, it's worth remembering when troubleshooting
that the log from `xine-server` will be a more useful source of information
about audio-related problems. `xine-server-x` is only a front-end to
`xine-server`, and doesn't do any audio playback of its own.

## Principle of operation

The diagram below shows the basic structure of the XSX/`xine-server`
system.

```


                               +-------------------+
                               |  Index (database) |
                               +---------|---------+
                                         |
                 +------------------+----|---+
  +----------+   |  Web interface   |        |
  |  Browser |---|  /gui/xxx        |        |    +--------------+ 
  |----------+   +------------------+        |    | Audio device |
  |JavaScript|---|  REST interface  |        |    +-|------------+
  +----------+   |  /api/xxx        |  XSX   |      |
                 +------------------+        |      |
                 |  Exernal files   |        |    +-|-----------+
                 |  /ext/xxx        |        |    | xine-server |
                 +------------------+        |    +-|-----------+
                 |  Internal files  |        |      |
                 |  /int/xxxx       |        |      |
                 +----------------------|--|-+      |
                         +--------------+  |        |
                         |                 |        |
                  +------|--------+   +----|--------|--+
                  | Radio station |   | Audio files    |
                  |    files      |   +----------------+
                  +---------------+


```

XSX provides a basic HTML interface, listing audio files on the local
filesystem, radio station files (in `gxsradio` format), and
entries from the index (a sqlite3 database). This index is built
by scanning metadata from the local audio files; scanning can be
full, or incremental. A full scan is exhaustive, and can take a 
very long time on older hardware or with larger music collections.

All basic browser requests have URIs of the form `/gui/...`

The HTML interface does not, by itself, carry out any audio 
operations on `xine-server`. These operations are the responsibility
of the REST interface. REST requests take arguments in the HTTP URI,
and respond with JSON data. In many cases the returned data will
just indicate success of failure, but some REST API functions return
extensive data (playlist contents, results of database searches).

All REST API requests have URIs that begin with `/api/...`

When a browser requests an HTML page, the respose will include 
a link to a JavaScript file `functions.js`, which provides the
interface between the browser and the REST API. For example, 
the JavaScript program starts a timer which triggers every second,
and issues the API request `/api/status`. This request returns
a JSON object that provides information about the item currently
playing, etc. The JavaScript program formats this information and
displays it on the web page. 

To play audio, XSX uses the `xine-server` network API to control
`xine-server`. The network API allows files to be added to
the `xine-server` playlist, and played -- this is essentially the limit of
functionality of `xine-server. On a specific system, `xine-server`
may have other clients, like the `gxsradio` radio stream player. The 
clients do not conflict with one another, because `xine-server` only
maintains one playlist, and only plays one item at a time.

XSX maintains all its HTML files and associated items (PNG images, etc)
internally -- nothing needs to be installed except the
`xine-server-x` binary. Requests for internal files have URIs
begining `/int/...`. However, when displaying audio titles in the
web interface, XSX will locate cover art images, if their are any
in the same directory as the audio files. These images are on
the filesystem, not part of the XSX binary (of course), and requests
for these have URIs begining `/ext/...`. 

The REST API is described in a separate document -- see 
`README.REST\_API`. 


## Notes

### Browser compatibility

XSX requires a fairly modern web browser. It uses media queries to
size the display to fit screens of different sizes, and 'flex' layout
to create tables with dynamic column numbers. Most of the testing
has been done using Mozilla firefox, although Google Chrome seems
to work fine. 

## Signals

`xine-server-x` traps the usual shut-down signals (SIGINT, etc) and
tries to shut down cleanly. Depending on what the server is doing at
the time, it might take a second or two to shut down -- perhaps longer
if it is in the process of making a network connection to an Internet
radio stream. If XSX started `xine-server`, it will try to stop it
if XSX shuts down on a signal. 

### Directory play order

When playing tracks by album, the track tag (if there is one) is
used to sort the play order. Most (all?) media tagging schemes have
no notion of a numeric track tag -- it's usually just a text string. 
This means that it's common to see tracks tagged as "1/20" or
"003 of 10", etc. XSX uses the integer cast feature in sqlite3 to
order the tracks by the database's notion of a number. This is
generally effective, but it certainly won't help if tracks are
tagged "track01" or "one". There isn't much that XSX can do about
freakish tagging schemes.

XSX can play all the files in a specified directory, rather than
(for example) all the tracks in an album. In this mode of operation.
XSX does not make use of the index (database) -- in fact, until an index
has been constructed this is the only method that XSX provides for
viewing and playing audio, so it's best if this operation is
completely independent of the index.

When playing files in a directory, because no metainfo is used,
files are played in alphanumeric order of filename. This is probably
not the order in which the album producer expected them to be
played.

### Meta-info display

XSX display the album, track, etc for the item that is currently
playing. This information comes from `xine-server`, via the
Xine library, and not the internal database. Xine's support for
tags is not particularly reliable, but XSX works this way, rather
than looking up meta-info in the database, because (a) it's
faster than repeatedly looking this up and (b) there's no guarantee
that XSX is the only client of `xine-server`. XSX needs to be
able to display meta-info even if `xine-server` is playing 
something that XSX did not request.

### Cover art display 

The XSX web interface will attempt to display cover art images alongside
albums, if it can find any. For a specific album, the cover art image
is a file with a well-defined name (`folder.png`, `cover.jpg`, etc) in
the same folder as the first track in the album. You can add these
files explicitly, or allow the file scanner to try to extract them
from the audio files (see below). XSX has no support for downloading
cover art files from Internet sources although, of course, you can
download them manually and store them in album folders along with the
audio files.

### Cover art extraction

Extracting cover art from audio files is time-consuming, so XSX does it
as a one-time operation as part of the audio scanning process. 
During scanning, the first file in a directory that has an embedded 
cover image is used as the cover source. The image is extracted and
stored in the same directory, with the name `cover` and extension
appropriate to the type of image stored in the audio file.

The cover art extraction process will not overwrite a cover image
if it already exists, whether it was extracted by the scanner or
added explicitly by the user.

### Volume slider

It is important to understand that the volume slider in the web interface
just sends a volume change command to `xine-server`. What `xine-server`
does with the command depends on its audio configuration. It may change
the system's global audio volume, or it may change a specific channel
for `xine-server`; see the `xine-server` documentation for more information
on volume handling. 

The volume slider does not reposition itself if the volume is changed
by some other software. If this happens, a small movement of the 
slider can appear to make a dramatic change in volume.

### Radio station files

By default, XSX looks for radio station files in the directory
`/usr/share/gxsradio`, for compatibility with `gxsradio`. This
location can be changed using the `--gxsradio` command-line switch
or `gxsradio` configuration file setting. 

#a## Launching `xine-server` from XSX

For convenience, `xine-server-x` can launch `xine-server` on startup,
and close it on shutdown. This action is not the default, because
`xine-server` can operate independently of `xine-server-x`, having other
(local or remote) clients. See the description below of `--xslaunch` for
more information.

### User privileges

`xine-server-x` is designed to run as an unprivileged user. It needs no
special permissions, because all the actual audio playback is handled by
`xine-server`. `xine-server` itself probably should also run as an 
unprivileged user, with only the necessary permissions to access the
audio device. To perform file scanning (for maintaining the index), 
the index file needs to be writeable. To extract cover art images
from audio files, the audio file directories need to be writable. 
XSX can be operated with a completely read-only filesystem -- as
might be required in an embedded system -- but some other provision
will have to be made to create the index.

## Author and legal

XSX is copyright (c)2020 Kevin Boone, and distributed in accordance
with the terms of the GNU Public Licence, v3.0. Essentially this
means that you may use this software in any way you see fit, so long
as source code continues to be made available, and the original
author is acknowledged. There is no warranty of any kind.

## Revision history

*Version 0.1, April 2020*<br/>
First functional release






