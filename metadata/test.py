from opentopia_webcam_metadata_scraper import WebcamMetadataScraper
from webcam_metadata_manager import WebcamMetadataManager


# Tests basic functionality of the webcam metadata manager.
def main():
  manager = WebcamMetadataManager('fake/path.db')
  manager.set_scraper(WebcamMetadataScraper)
  data = manager.get('11008')
  assert(data)
  manager.set_scraper(None)
  assert(data == manager.get('11008', 'opentopia'))
  assert(manager.get('11008') is None)
  print("All assertions passed.")


if __name__ == "__main__":
  main()
