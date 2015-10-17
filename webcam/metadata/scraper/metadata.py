class Metadata(object):
  """A data class used to store webcam metadata.

  Attributes:
    source (string): The source used to find the metadata.
    identifier (string): The source's unique identifier for this webcam.
    livestill_url (string): The URL needed to fetch an up-to-date still image
        from the webcam.
    is_live (bool): True if and only if it is possible to retrieve data from
        the webcam.
    facility (string): A human-readable description of the facility in which
        the webcam is located.
    city (string): The city the webcam is located.
    country (string): The country the webcam is located.
    region (string): The region/state the webcam is located.
    brand (string): The brand of webcam.
    coordinates (string): The coordinates of the webcam.
  """
  _SUPPORTED_ATTRIBUTES = [
      'source',
      'identifier',
      'livestill_url',
      'is_live',
      'facility',
      'city',
      'country',
      'region',
      'brand',
      'coordinates'
  ]


  def __init__(self, metadata):
    """Initialize the webcam metadata.

    Extracts supported attributes from the metadata into the object. If a
    supported attribute is not present in metadata, the attribute is set to
    None.

    Args:
      metadata (dict): Mapping from attribute names to values.
    """
    for attr in Metadata._SUPPORTED_ATTRIBUTES:
      setattr(self, attr, metadata.get(attr))


  def __str__(self):
    """The string representation of this metadata.

    Used for printing Metadata objects.
    """
    metadata = {}
    for attr in Metadata._SUPPORTED_ATTRIBUTES:
      value = getattr(self, attr, None)
      if value is not None:
        metadata[attr] = value
    return str(metadata)
