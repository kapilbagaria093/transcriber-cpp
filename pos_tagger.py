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

    # Ignore whitespace tokens
    if token.is_space:
        continue

    print(
        f"{token.text}\t"
        f"{token.lemma_}\t"
        f"{token.pos_}\t"
        f"{token.tag_}\t"
        f"{token.morph}\t"
        f"{token.dep_}\t"
        f"{token.head.i}\t"
        f"{token.ent_type_}\t"
        f"{token.ent_iob_}\t"
        f"{token.i}\t"
        f"{token.idx}\t"
        f"{int(token.like_num)}\t"
        f"{int(token.is_punct)}"
    )