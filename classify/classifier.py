import caffe
import numpy as np
import os

def my_directory():
  return os.path.dirname(os.path.realpath(__file__))

class Classifier(object):
  def __init__(self):
    # We will always use the GPU. This must be set BEFORE creating the net.
    caffe.set_device(0)
    caffe.set_mode_gpu()

    self.net_ = caffe.Net(self.model_path(), self.weights_path(), caffe.TEST)
    self.transformer_ = self.create_transformer()
    self.labels_ = self.load_labels()



  # caffe.io.load_image(path) returns the following:
  #    image : an image with type np.float32 in range [0, 1]
  #        of size (H x W x 3) in RGB or
  #        of size (H x W x 1) in grayscale.
  def classify(self, image):
    normalized = image.astype(np.float32) / image.max()
    self.net_.blobs['data'].data[...] = self.transformer_.preprocess('data', normalized)
    out = self.net_.forward()
    return out['prob'][0].argmax()

  def rankings(self, image):
    normalized = image.astype(np.float32) / image.max()
    self.net_.blobs['data'].data[...] = self.transformer_.preprocess('data', normalized)
    out = self.net_.forward()
    probabilities = out['prob'][0]
    ordered  = out['prob'][0].argsort()
    infos = [(self.label(o), o, out['prob'][0][o]) for o in reversed(ordered)]
    return infos

  # Returns a human-readable label for the given classification.
  def label(self, classification):
    return self.labels_[classification]

  # abstract (required)
  def create_transformer(self):
    pass

  # abstract (required)
  def model_path(self):
    pass

  # abstract (required)
  def weights_path(self):
    pass

  # abstract (optional)
  def set_batch_size(self, batch_size):
    pass

  # abstract (optional)
  def load_labels(self):
    pass

class ReferenceClassifier(Classifier):
  def __init__(self):
    super(ReferenceClassifier, self).__init__()

  def create_transformer(self):
    t = caffe.io.Transformer({'data': self.net_.blobs['data'].data.shape})
    t.set_transpose('data', (2,0,1))
    mean_path = my_directory() + '/imagenet/ilsvrc_2012_mean.npy'
    t.set_mean('data', np.load(mean_path).mean(1).mean(1))
    t.set_raw_scale('data', 255)
    t.set_channel_swap('data', (2,1,0))
    return t

  def model_path(self):
    return my_directory() + '/models/bvlc_reference_caffenet/deploy.prototxt'

  def weights_path(self):
    return my_directory() + '/models/bvlc_reference_caffenet/bvlc_reference_caffenet.caffemodel'

  def set_batch_size(self, batch_size):
    self.net_.blobs['data'].reshape(batch_size, 3, 227, 227)

  def load_labels(self):
    return np.loadtxt(my_directory() + '/ilsvrc12/synset_words.txt', str,
        delimiter='\t')

class CamClassifier(Classifier):
  def __init__(self):
    super(CamClassifier, self).__init__()

  def create_transformer(self):
    t = caffe.io.Transformer({'data': self.net_.blobs['data'].data.shape})
    t.set_transpose('data', (2,0,1))
    mean_path = my_directory() + '/imagenet/ilsvrc_2012_mean.npy'
    t.set_mean('data', np.load(mean_path).mean(1).mean(1))
    t.set_raw_scale('data', 255)
    # t.set_channel_swap('data', (2,1,0))
    return t

  def model_path(self):
    return my_directory() + '/models/camnet2/deploy.prototxt'

  def weights_path(self):
    return my_directory() + '/models/camnet2/net2.caffemodel'

  def set_batch_size(self, batch_size):
    self.net_.blobs['data'].reshape(batch_size, 3, 227, 227)

  def load_labels(self):
    return ['animal', 'human', 'noise', 'vehicle'] + ['NA'] * 16
