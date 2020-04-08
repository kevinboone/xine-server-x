### Replace Xine values for audio tags with local values from the index

...rather than just displaying the tag info returned by `xine-server`.
`xine-server` gets its meta-info from Xine, which is not particularly reliable
in this area.

[The problem is that, on some occasions, `xine-server` could be playing
something requested by a differnet client than XSX, so the may be no
possibility of even knowing whether there is better meta-info in XSX's
database.]

Urgency: low, difficulty: probably impossible, unless many simplifying
assumptions are made 

### Remove [play] link on directories with no audio files. 

[This will tidy things up, but will generate a _lot_ of extra work for the
server in some set-ups]

Urgency: low, difficulty: moderate

### Seek control

[Not of overwhelming useful for music, but more important for spoken-word
content, that tends to be much longer.]

Urgency: moderate, difficulty: moderate

### Add missing REST API functions. 

There is a great deal that the web interface can do that is not exposed as REST
functions. This will make it difficult to implement other, non-web clients.

[The problem here is that, until some clients are implemented, it isn't 
all that clear what functionality the REST API should expose.]

Urgency: unknown, difficulty: unknown

### Implement a proper log file 

When running in the background, it is difficult to log detailed information
because it all goes to syslog. It might also be useful to have an access log,
which would have to rotate. 

[The problem with this lies in synchronizing threads, without creating a
bottleneck. We wouldn't want a log file where lines consist of mixed
content from different threads.]

Urgency: low, difficulty: moderate

### Allow user to adjust the number of albums, genres, etc., 

...shown on each search results page. 

[It's not even clear how to do this, because the web user interface is
completely stateless. We could add a control associated with the 'limit' URI
argument on each page, rather than propating this parameter from request URI
to HTML page]

Urgency: moderate, difficulty: unknown; probably hard, without significant architectural changes

### Add album image on tracks page

[The complication here is that the tracks page can show general search
results, as well as album tracks. So the tracks on the tracks page might not
come from the same album. We need to add logic to check this, and then add the
album image if it is appropriate for all listed tracks (some of which might
not be on the same results page). A dirty solution would be to add a specific
album request argument for situations where the track list comes from an
album, which would indicate the albums whose cover art to use.]

Urgency: moderate, difficulty: hard, without nasty kludges

### Consider adding genre, artist, composer to search results. 

[This is easy to do, but is it useful?]

Urgency: low, difficulty: easy

### Highlight current item in playlist 

[This is currently very difficult, without having the playlist page poll the
server for transport changes]

Urgency: moderate, difficulty: hard

### Investigate full-text search support in sqlite3

... or in some other way extend the "contains" test so it matches on word
boundaries other than spaces.

Urgency: moderate, difficulty: unknown

### Break up files page into smaller sections

...as all the database search pages are. 

[With a flat directory structure, building this page can be very
time-consuming, and use a lot of memory -- as well as potentially transferring
a huge number of cover-art images to the browser. This would require replacing
the built-in directory expansion logic with new functions that retrieve
directory content in blocks, and adding page navigation controls as the
database searches have.]

Urgency: moderate, difficulty: hard

### Shuffle playlist

Urgency: low, difficulty: easy

### "Play something" feature

Add a feature to play a random selection of tracks from the database. 
This could probably also be done from the filesystem, for situations 
where there is no database, but prolonged non-database operation isn't
really envisaged

Urgency: moderate, difficulayt: low

### Home page

At present, invoking the web interface without a URI shows the albums
page if there is a database, and the files page if not. It might
be better to have a separate home page, with access to frequently-used
features. 

Urgency: low, difficult: low





