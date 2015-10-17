import scraper.opentopia
import manager


# Tests basic functionality of the webcam metadata manager.
def main():
  m = manager.Manager('metadata.p')
  m.set_scraper(scraper.opentopia.Scraper)
  data = m.get('11008')
  assert(data)
  m.set_scraper(None)
  assert(data == m.get('11008', 'opentopia'))
  assert(m.get('11008') is None)
  print("All assertions passed.")


if __name__ == "__main__":
  main()
