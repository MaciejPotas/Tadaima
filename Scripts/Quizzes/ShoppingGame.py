import random

def play_shopping_game():
    print("Witaj w grze w sklep!")
    print("Wpisz 'q', aby zakończyć grę.\n")
    
    while True:
        
        # Predefiniowane produkty w sklepie i ich ceny
        items = {
            'Ksiazka: ほん (hon)': random.randint(0, 99), 
            'Talerz: さら (sara)': random.randint(0, 99), 
            'Krzeslo: いす (isu)': random.randint(0, 99), 
            'Torba: かばん (kaban)': random.randint(0, 99),  
            'Wazon: かびん (kabin)': random.randint(0, 99),
            'Portfel: さいふ (saifu)': random.randint(0, 99), 
            'Samochód: くるま (kuruma)': random.randint(0, 99),
        }
    
        selected_products = selected_items = random.sample(list(items.keys()), 4)
        
        print("Dzis w promocji mamy:")
        for item_name in selected_products:
            print(f"{item_name.capitalize()} - {items[item_name]}")
        
        selected_index = input("")
       
        if selected_index.lower() == 'q':
            continue
            break

play_shopping_game()
