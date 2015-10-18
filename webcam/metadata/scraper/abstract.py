import abc


class Scraper(object, metaclass=abc.ABCMeta):
  """An abstract interface for scraping metadata from webcams."""
  @staticmethod
  @abc.abstractmethod
  def scrape(identifier):
    """Scrapes and returns the metadata for the webcam.

    Args:
      identifier (str): A unique identifier for the webcam.

    Returns:
      WebcamMetadata: Metadata for the webcam.
    """
    raise NotImplementedError


  @staticmethod
  @abc.abstractmethod
  def source():
    """The source for this scraper.

    Returns:
      string: Canonical name for where this scraper gets its information.
    """
    raise NotImplementedError
