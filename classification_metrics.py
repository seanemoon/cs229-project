import argparse
import cv2

from classify.classifier import CamClassifier

K_LABELS = ['animal', 'human', 'noise', 'vehicle']
K_LABEL_INDEX = {label:i for i, label in enumerate(K_LABELS)}

def retrieve_arguments():
  parser = argparse.ArgumentParser()
  parser.add_argument("dataset", help="Text file containing list of patches")
  args = parser.parse_args()
  return args

def filter_rankings(rankings):
  labels_of_interest = set(('human', 'noise', 'vehicle'))
  prob_of_label = {}
  for label, idx, prob in rankings:
    if label in labels_of_interest:
      prob_of_label[label] = prob
  return prob_of_label

def classify(classifier, patch):
  rankings = classifier.rankings(patch)
  prob_of_label = filter_rankings(rankings)
  if prob_of_label['noise'] < 1.0 - 0.95:
    if prob_of_label['human'] > 0.6*prob_of_label['vehicle']:
      return 'human'
    else:
      return 'vehicle'
  else:
    return 'noise'

def main(args):
  classifier = CamClassifier()
  classifier.set_batch_size(1)  # TODO: Batch if its running to slowly.

  with open(args.dataset) as dataset:
    labeled_patches = [line.split() for line in dataset]
    labeled_patches = [(x[0], int(x[1])) for x in labeled_patches]

  num_labels = {label: 0 for label in K_LABELS}
  num_classified = {label: 0 for label in K_LABELS}
  num_correct = {label: 0 for label in K_LABELS}
  num_present = {label: 0 for label in K_LABELS}

  confusion = {}
  for label in K_LABELS:
    confusion[label] = {}
    for label2 in K_LABELS:
      confusion[label][label2] = 0

  counter = 0
  for patch, label in labeled_patches:
    print patch
    counter += 1
    # print counter
    patch = cv2.imread(patch, cv2.IMREAD_COLOR)
    label = K_LABELS[label]
    if patch is None:
      continue
    classification = classify(classifier, patch)

    confusion[classification][label] += 1

    print label, classification
    num_labels[label] += 1
    num_classified[classification] += 1
    if label == classification:
      num_correct[classification] += 1
    if classification != 'noise':
      num_present[label] += 1


  print num_correct
  print num_classified
  print num_labels
  for label in K_LABELS:
    num_classified[label] = max(1, num_classified[label])
    num_labels[label] = max(1, num_labels[label])

  precision = {l: num_correct[l] / float(num_classified[l]) for l in K_LABELS}
  recall = {l: num_correct[l] / float(num_labels[l]) for l in K_LABELS}

  for label in K_LABELS:
    print ("%s\n\tprecision: %f\n\trecall: %f" % (label, precision[label], recall[label]))

  num_present['animal'] = 0
  total_present = sum(num_present.values())
  for label in K_LABELS:
    print "%s filter precentage: %f" % (label, num_present[label]/float(total_present))

  print confusion


if __name__ == "__main__":
  args = retrieve_arguments()
  main(args)
