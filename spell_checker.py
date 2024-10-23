from textblob import TextBlob

def correct_spelling(text):
    blob = TextBlob(text)
    corrected_text = blob.correct()
    return str(corrected_text)

# Example usage
input_text = "This is a smple text with som mistkes."
corrected_text = correct_spelling(input_text)
print(f"Original: {input_text}")
print(f"Corrected: {corrected_text}")
