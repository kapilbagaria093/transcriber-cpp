import sys
import spacy

nlp = spacy.load("en_core_web_sm")

if len(sys.argv) != 2:
    print("Usage: python pos_tagger.py <input_file>", file=sys.stderr)
    sys.exit(1)

input_file = sys.argv[1]

with open(input_file, "r", encoding="utf-8") as f:
    text = f.read()

doc = nlp(text)

for token in doc:
    if token.is_space:
        continue

    print(f"{token.text}\t{token.tag_}\t{token.lemma_}")