import requests
from bs4 import BeautifulSoup
import random
import time

def fetch_insights_from_witandfolly():
    url = 'https://witandfolly.co/100-incredibly-interesting-facts-about-japan/'
    insights = []

    try:
        response = requests.get(url)
        response.raise_for_status()
    except requests.RequestException as e:
        print(f"Error fetching Wit and Folly URL: {e}")
        return insights

    soup = BeautifulSoup(response.content, 'html.parser')
    article = soup.find('div', {'class': 'entry-content'})
    if article:
        paragraphs = article.find_all('p')
        for paragraph in paragraphs:
            text = paragraph.get_text(strip=True)
            if text and not any(phrase in text.lower() for phrase in ['affiliate', 'disclosure', 'sponsor', 'click here', 'read more']):
                insights.append(text)
    
    return insights

def get_cultural_insight():
    insights = fetch_insights_from_witandfolly()

    if not insights:
        return "No Insights Found. Unable to fetch cultural insights at this time."

    selected_insight = random.choice(insights)
    return selected_insight

if __name__ == '__main__':
    insight = get_cultural_insight()
    print(insight)
