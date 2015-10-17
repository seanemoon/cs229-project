from abstract_webcam_metadata_scraper import AbstractWebcamMetadataScraper
from webcam_metadata import WebcamMetadata

from lxml import html
import logging
import urllib.error
import urllib.parse
import urllib.request


class WebcamMetadataScraper(AbstractWebcamMetadataScraper):
  """Scrapes metadata from webcams on Opentopia"""


  @staticmethod
  def scrape(identifier):
    """Scrapes and returns the metadata for the webcam.

    Args:
      identifier (str): A unique identifier for the webcam.

    Returns:
      WebcamMetadata: Metadata for the webcam.
    """
    try:
      response = urllib.request.urlopen(
          WebcamMetadataScraper.construct_request(identifier))
      metadata = WebcamMetadataScraper.extract_metadata(response.read())
    except urllib.error.URLError as error:
      logger = logging.getLogger('opentopia_webcam_metadata_scraper')
      logger.error('failed to scrape metadata for %s.' % identifier)
      metadata = WebcamMetadata({'is_live': False})

    metadata.identifier = identifier
    metadata.source = WebcamMetadataScraper.source()
    return metadata


  @staticmethod
  def source():
    """The source for this scraper.

    Returns:
      string: Canonical name for where this scraper gets its information.
    """
    return "opentopia"


  @staticmethod
  def construct_request(identifier):
    """Constructs a request for the webcam identifier

    Args:
      identifier (string): A unique identifier for the webcam.

    Returns:
      urllib.request.Request: A request for the opentopia webpage to scrape.
    """
    base_url = 'http://www.opentopia.com/webcam/'
    options = {'viewmode': 'livestill'}
    url = ''.join([base_url, identifier, '?', urllib.parse.urlencode(options)])
    return urllib.request.Request(url)


  @staticmethod
  def parse_caminfo_elem(caminfo_elem):
    """Parses a caminfo element containing various metadata.

    Args:
      caminfo_elem (Element): the HTML element containing webcam info.

    Returns:
      list: List of tuples containing webcam metadata.

      Each tuple is of the form (attribute, value).
    """
    metadata_tuples = []
    for child in list(caminfo_elem):
      for label in list(child):
        if 'left' in label.get('class'):
          key = label.text.strip(': ').lower()
        elif 'right' in label.get('class'):
          if 'geo' in label.get('class'):
            for coordinate in list(label):
              key = coordinate.get('class')
              value = coordinate.text.strip()
              metadata_tuples.append((key, value))
              break
          else:
            value = label.text.strip().lower()
      metadata_tuples.append((key, value))
    return metadata_tuples


  @staticmethod
  def extract_metadata(page):
    """Extracts and returns metadata about the page's webcam.

    Args:
      page (bytes): An HTML page holding information about a webcam.

    Returns:
      WebcamMetadata:  Metadata for the webcam.
    """
    tree = html.fromstring(page)
    livestill_xpath = '//*[@id="stillimage"]'
    caminfo_xpath = '//*[@id="caminfo"]'
    livestill_elems = tree.xpath(livestill_xpath)
    caminfo_elems = tree.xpath(caminfo_xpath)

    if caminfo_elems:
      caminfo = WebcamMetadataScraper.parse_caminfo_elem(caminfo_elems[0])
    else:
      caminfo = []

    if livestill_elems:
      livestill_url = livestill_elems[0].get('src')
    else:
      livestill_url = ""

    metadata = {}
    for key, value in caminfo:
      metadata[key] = value
    metadata['livestill_url'] = livestill_url
    metadata['is_live'] = livestill_url and b'trouble contacting' not in page

    return WebcamMetadata(metadata)
