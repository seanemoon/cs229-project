Contains executables for scraping and processing webcam data from the internet.


## scrape_metadata.py
Scrapes metadata associated with webcams from sources.
To use this executable, specify the following:
- source: The name of the source of the webcams.
- identifiers: A range of unique identifiers for the webcams.


## scrape_frames.py
Scrapes frames associated with webcams from sources.
To use this executable, specifiy the following:
- source: The name of the source of the webcams.
- identifiers: A range of unique identifiers for the webcams.
- period: The period (in minutes) between each frame.
- threads: The number of scraping threads to use.


## list_metadata.py
Lists metadata associated with scraped webcams.
You may specify the following flag:
- live: Only shows metadata for live webcams.
