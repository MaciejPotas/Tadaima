import random
import sys
import time

# Set the default encoding to UTF-8 if possible
try:
    sys.stdout.reconfigure(encoding='utf-8')
except AttributeError:
    # Reconfigure might not be available in older Python versions, so we skip it
    pass

def convert_to_te_form(verb):
    """
    Convert a given verb to its -te form. Handles regular verbs, exceptions, and irregular verbs.
    """
    if verb.endswith('る'):
        if len(verb) > 2 and verb[-3] in ['え', 'け', 'せ', 'て', 'ね', 'へ', 'め', 'れ', 'げ', 'ぜ', 'で', 'べ', 'ぺ']:
            return verb[:-1] + 'て'  # Ru-verb (ichidan verb)
        else:
            return verb[:-1] + 'って'  # U-verb ending with 'る'
    elif verb.endswith('う') or verb.endswith('つ') or verb.endswith('る'):
        return verb[:-1] + 'って'
    elif verb.endswith('む') or verb.endswith('ぶ') or verb.endswith('ぬ'):
        return verb[:-1] + 'んで'
    elif verb.endswith('く'):
        if verb == 'いく':
            return 'いって'  # Exception for "to go"
        else:
            return verb[:-1] + 'いて'
    elif verb.endswith('ぐ'):
        return verb[:-1] + 'いで'
    elif verb.endswith('す'):
        return verb[:-1] + 'して'
    elif verb == 'する':
        return 'して'  # Irregular verb
    elif verb == 'くる':
        return 'きて'  # Irregular verb
    else:
        return verb  # No change

def get_predefined_verbs():
    """
    Return a predefined list of common Japanese verbs with their meanings and proper -te forms.
    """
    verbs = [
    ('ののしる', 'to abuse verbally', 'ののして'),
    ('なじむ', 'to become familiar with', 'なじんで'),
    ('たす', 'to add', 'たして'),
    ('くわえる', 'to add to', 'くわえて'),
    ('こたえる', 'to answer', 'こたえて'),
    ('つく', 'to arrive', 'ついて'),
    ('こころざす', 'to aspire', 'こころざして'),
    ('おそう', 'to attack', 'おそって'),
    ('めだつ', 'to stand out', 'めだって'),
    ('さける', 'to avoid', 'さけて'),
    ('やく', 'to bake', 'やいて'),
    ('きんじる', 'to forbid', 'きんじて'),
    ('いる', 'to need', 'いって'),
    ('ふける', 'to grow old', 'ふけて'),
    ('いきる', 'to live', 'いきて'),
    ('きこえる', 'to be heard', 'きこえて'),
    ('もとづく', 'to be based on', 'もとづいて'),
    ('くらむ', 'to be clouded', 'くらんで'),
    ('うまれる', 'to be born', 'うまれて'),
    ('おれる', 'to break', 'おれて'),
    ('こげる', 'to burn', 'こげて'),
    ('なおる', 'to be cured', 'なおって'),
    ('きまる', 'to be decided', 'きまって'),
    ('ことなる', 'to differ', 'ことなって'),
    ('ちがう', 'to be different', 'ちがって'),
    ('よっぱらう', 'to get drunk', 'よっぱらって'),
    ('たりる', 'to be enough', 'たりて'),
    ('どきどきする', 'to be excited', 'どきどきして'),
    ('みつかる', 'to be found', 'みつかって'),
    ('たまぎる', 'to be astonished', 'たまぎって'),
    ('よろこぶ', 'to be glad', 'よろこんで'),
    ('こまる', 'to be troubled', 'こまって'),
    ('おくれる', 'to be late', 'おくれて'),
    ('なまける', 'to be lazy', 'なまけて'),
    ('なくなる', 'to be lost', 'なくなって'),
    ('まじる', 'to be mixed', 'まじって'),
    ('つぐ', 'to be next', 'ついで'),
    ('きんちょうする', 'to be nervous', 'きんちょうして'),
    ('もてる', 'to be popular', 'もてて'),
    ('もうかる', 'to be profitable', 'もうかって'),
    ('まよう', 'to be puzzled', 'まよって'),
    ('よみがえる', 'to revive', 'よみがえって'),
    ('かなしむ', 'to be sad', 'かなしんで'),
    ('たまる', 'to accumulate', 'たまって'),
    ('ぬめる', 'to be slippery', 'ぬめって'),
    ('おどろく', 'to be surprised', 'おどろいて'),
    ('みえる', 'to be visible', 'みえて'),
    ('うらむ', 'to hold a grudge', 'うらんで'),
    ('なる', 'to become', 'なって'),
    ('かすむ', 'to become hazy', 'かすんで'),
    ('こむ', 'to become crowded', 'こんで'),
    ('かげる', 'to become dim', 'かげって'),
    ('かわく', 'to become dry', 'かわいて'),
    ('すく', 'to become empty', 'すいて'),
    ('いたむ', 'to become hurt', 'いたんで'),
    ('はやる', 'to become popular', 'はやって'),
    ('つよまる', 'to become stronger', 'つよまって'),  
    ('ぬれる', 'to become wet', 'ぬれて'),
    ('はじまる', 'to begin', 'はじまって'),
    ('しんじる', 'to believe', 'しんじて'),
    ('みくびる', 'to despise', 'みくびって'),
    ('ひねる', 'to twist', 'ひねって'),
    ('かむ', 'to bite', 'かんで'),
    ('さく', 'to bloom', 'さいて'),
    ('ふく', 'to blow', 'ふいて'),
    ('のる', 'to board', 'のって'),
    ('いばる', 'to boast', 'いばって'),
    ('かりる', 'to borrow', 'かりて'),
    ('こわれる', 'to break', 'こわれて'),
    ('こうきゅうする', 'to breathe deeply', 'こうきゅうして'),
    ('きれる', 'to be cut', 'きれて'),
    ('たてる', 'to build', 'たてて'),
    ('やける', 'to be burned', 'やけて'),
    ('かう', 'to buy', 'かって'),
    ('よぶ', 'to call', 'よんで'),
    ('でんわする', 'to call by phone', 'でんわして'),
    ('できる', 'to be able', 'できて'),
    ('きをつける', 'to be careful', 'きをつけて'),
    ('かまう', 'to care about', 'かまって'),
    ('はこぶ', 'to carry', 'はこんで'),
    ('かつぐ', 'to carry on one\'s shoulders', 'かついで'),
    ('はさむ', 'to pinch', 'はさんで'),
    ('いわう', 'to celebrate', 'いわって'),
    ('かえる', 'to change', 'かえて'),
    ('かわる', 'to be changed', 'かわって'),
    ('おう', 'to chase', 'おって'),
    ('しゃべる', 'to chat', 'しゃべって'),
    ('しらべる', 'to check', 'しらべて'),
    ('かむ', 'to chew', 'かんで'),
    ('えらぶ', 'to choose', 'えらんで'), 
    ('くいちがう', 'to clash', 'くいちがって'),
    ('はれる', 'to clear up', 'はれて'),
    ('のぼる', 'to climb', 'のぼって'),
    ('しまる', 'to close', 'しまって'),
    ('くずれる', 'to collapse', 'くずれて'),
    ('くる', 'to come', 'きて'),
    ('はずれる', 'to come off', 'はずれて'),
    ('ほどける', 'to come untied', 'ほどけて'),
    ('なぐさめる', 'to console', 'なぐさめて'),
    ('いいつける', 'to command', 'いいつけて'),
    ('かよう', 'to commute', 'かよって'),
    ('くらべる', 'to compare', 'くらべて'),
    ('もんくをいう', 'to complain', 'もんくをいって'),
    ('こらす', 'to concentrate', 'こらして'),
    ('つなぐ', 'to connect', 'つないで'),  
    ('れんらくする', 'to contact', 'れんらくして'),
    ('つづける', 'to continue', 'つづけて'),
    ('つづく', 'to continue (intransitive)', 'つづいて'),
    ('りょうりする', 'to cook', 'りょうりして'),
    ('ひやす', 'to cool', 'ひやして'),
    ('ひえる', 'to be cooled', 'ひえて'),
    ('かぞえる', 'to count', 'かぞえて'),
    ('わたる', 'to cross', 'わたって'),
    ('こす', 'to cross over', 'こして'),
    ('くだく', 'to crush', 'くだいて'),
    ('なく', 'to cry', 'ないて'),
    ('きる', 'to cut', 'きって'),
    ('おどる', 'to dance', 'おどって'),
    ('つきあう', 'to date', 'つきあって'),
    ('くさる', 'to decay', 'くさって'),
    ('いつわる', 'to deceive', 'いつわって'),
    ('きめる', 'to decide', 'きめて'),    
    ('さだめる', 'to determine', 'さだめて'),
    ('かざる', 'to decorate', 'かざって'),
    ('へる', 'to decrease', 'へって'),
    ('まかす', 'to defeat', 'まかして'),
    ('はいたつする', 'to deliver', 'はいたつして'),
    ('たよる', 'to depend', 'たよって'),
    ('いいあらわす', 'to express', 'いいあらわして'),
    ('くずす', 'to demolish', 'くずして'),
    ('ほろぼす', 'to destroy', 'ほろぼして'),
    ('しぬ', 'to die', 'しんで'),
    ('ほる', 'to dig', 'ほって'),
    ('はなしあう', 'to discuss', 'はなしあって'),
    ('いやがる', 'to dislike', 'いやがって'),
    ('わける', 'to divide', 'わけて'),
    ('りこんする', 'to divorce', 'りこんして'),
    ('する', 'to do', 'して'),
    ('やる', 'to do', 'やって'),
    ('がんばる', 'to do one\'s best', 'がんばって'),
    ('だうんろーどする', 'to download', 'だうんろーどして'),
    ('うたがう', 'to doubt', 'うたがって'),
    ('かく', 'to draw', 'かいて'),
    ('のむ', 'to drink', 'のんで'),
    ('うんてんする', 'to drive', 'うんてんして'),
    ('かわかす', 'to dry', 'かわかして'),
    ('ほす', 'to air-dry', 'ほして'),
    ('たべる', 'to eat', 'たべて'),
    ('ねじる', 'to twist', 'ねじって'),
    ('めーるする', 'to email', 'めーるして'),
    ('だく', 'to embrace', 'だいて'),
    ('はげます', 'to encourage', 'はげまして'),
    ('おわる', 'to end', 'おわって'),
    ('たのしむ', 'to enjoy', 'たのしんで'),
    ('はいる', 'to enter', 'はいって'),
    ('すぐれる', 'to excel', 'すぐれて'),
    ('うんどうする', 'to exercise', 'うんどうして'),
    ('とりかえる', 'to exchange', 'とりかえて'),
    ('きたいする', 'to expect', 'きたいして'),
    ('めにあう', 'to experience', 'めにあって'),
    ('せつめいする', 'to explain', 'せつめいして'),
    ('だす', 'to extract', 'だして'),
    ('むかう', 'to face', 'むかって'),
    ('しっぱいする', 'to fail', 'しっぱいして'),
    ('ねむる', 'to fall asleep', 'ねむって'),
    ('ころぶ', 'to fall down', 'ころんで'),
    ('たおれる', 'to fall over', 'たおれて'),
    ('ふる', 'to fall (rain)', 'ふって'),
    ('ほれる', 'to fall in love', 'ほれて'),
    ('さがる', 'to fall, to drop', 'さがって'),
    ('おちる', 'to fall, to drop (intransitive)', 'おちて'),
    ('おならをする', 'to fart', 'おならをして'),
    ('こわがる', 'to fear', 'こわがって'),
    ('おそれる', 'to fear', 'おそれて'),
    ('めいる', 'to feel depressed', 'めいって'),
    ('しずむ', 'to sink', 'しずんで'),
    ('ほてる', 'to feel hot', 'ほてって'),
    ('かんじる', 'to feel', 'かんじて'),
    ('いじる', 'to fiddle with', 'いじって'),
    ('たたかう', 'to fight', 'たたかって'),
    ('みつける', 'to find', 'みつけて'),
    ('おわる', 'to finish', 'おわって'),
    ('かたづける', 'to clean up, to put away', 'かたづけて'),
    ('つる', 'to fish', 'つって'),
    ('うかぶ', 'to float', 'うかんで'),
    ('ながれる', 'to flow', 'ながれて'),
    ('たたむ', 'to fold', 'たたんで'),
    ('わすれる', 'to forget', 'わすれて'),
    ('ゆるす', 'to forgive', 'ゆるして'),
    ('ならぶ', 'to line up', 'ならんで'),
    ('あつまる', 'to gather', 'あつまって'),
    ('あおぐ', 'to fan', 'あおいで'),
    ('える', 'to get, to acquire', 'えて'),
    ('おこる', 'to get angry', 'おこって'),
    ('きかえる', 'to change clothes', 'きかえて'),
    ('くもる', 'to become cloudy', 'くもって'),
    ('ふとる', 'to get fat', 'ふとって'),
    ('こえる', 'to exceed', 'こえて'),
    ('いる', 'to exist (living things)', 'いて'),
    ('おりる', 'to get off', 'おりて'),
    ('つかれる', 'to get tired', 'つかれて'),
    ('おきる', 'to get up', 'おきて'),
    ('なれる', 'to get used to', 'なれて'),
    ('あたたまる', 'to warm oneself', 'あたたまって'),
    ('あげる', 'to give', 'あげて'),
    ('くれる', 'to give (me)', 'くれて'),
    ('かえす', 'to return something', 'かえして'),
    ('おもいきる', 'to give up', 'おもいきって'),
    ('にらむ', 'to glare at', 'にらんで'),
    ('かじる', 'to gnaw', 'かじって'),
    ('いく', 'to go', 'いって'),
    ('すすむ', 'to advance', 'すすんで'),
    ('くるう', 'to go mad', 'くるって'),
    ('でる', 'to go out', 'でて'),
    ('でかける', 'to go out (for a purpose)', 'でかけて'),
    ('まわる', 'to turn', 'まわって'),
    ('あがる', 'to go up', 'あがって'),
    ('にぎる', 'to grasp', 'にぎって'),
    ('むかえる', 'to greet, to receive', 'むかえて'),
    ('はえる', 'to grow, to sprout', 'はえて'),
    ('くれる', 'to get dark', 'くれて'),
    ('としとる', 'to grow old', 'としとって'),
    ('しずまる', 'to become quiet', 'しずまって'),
    ('しげる', 'to grow thick', 'しげって'),
    ('そだつ', 'to grow up', 'そだって'),
    ('ぐちる', 'to grumble', 'ぐちって'),
    ('わたす', 'to hand over', 'わたして'), 
    ('ある', 'to exist (inanimate objects)', 'あって'),
    ('もつ', 'to hold', 'もって'),
    ('きく', 'to listen, to ask', 'きいて'),
    ('たすける', 'to help', 'たすけて'),
    ('てつだう', 'to assist', 'てつだって'),
    ('かくれる', 'to hide (intransitive)', 'かくれて'),
    ('かくす', 'to hide (something)', 'かくして'),
    ('ほのめかす', 'to hint', 'ほのめかして'),
    ('やとう', 'to hire', 'やとって'),
    ('かかえる', 'to hold', 'かかえて'),
    ('いそぐ', 'to hurry', 'いそいで'),
    ('いためる', 'to hurt', 'いためて'),
    ('そうぞうする', 'to imagine', 'そうぞうして'),
    ('ふえる', 'to increase', 'ふえて'),
    ('しめす', 'to indicate', 'しめして'),
    ('くるしめる', 'to torment', 'くるしめて'),
    ('さそう', 'to invite', 'さそって'),
    ('やじる', 'to jeer', 'やじって'),
    ('じょうだんをいう', 'to joke', 'じょうだんをいって'),
    ('とぶ', 'to jump', 'とんで'),
    ('かう', 'to buy', 'かって'),
    ('ける', 'to kick', 'けって'),
    ('ころす', 'to kill', 'ころして'),
    ('きすする', 'to kiss', 'きすして'),
    ('しる', 'to know', 'しって'),
    ('ちゃくりくする', 'to land', 'ちゃくりくして'),
    ('わらう', 'to laugh', 'わらって'),
    ('ふせる', 'to lay face down', 'ふせて'),
    ('つれる', 'to lead', 'つれて'),
    ('もらす', 'to leak (something)', 'もらして'),
    ('もれる', 'to leak', 'もれて'),
    ('かたむく', 'to lean', 'かたむいて'),
    ('まなぶ', 'to learn', 'まなんで'),
    ('ならう', 'to learn (from someone)', 'ならって'),
    ('のこす', 'to leave (something)', 'のこして'),
    ('のこる', 'to remain', 'のこって'),
    ('かす', 'to lend', 'かして'),
    ('なめる', 'to lick', 'なめて'),
    ('うそをいう', 'to lie', 'うそをいって'),
    ('かぎる', 'to limit', 'かぎって'),
    ('ならべる', 'to line up (something)', 'ならべて'),
    ('くらす', 'to live', 'くらして'),
    ('すむ', 'to live, to reside', 'すんで'),
    ('みる', 'to see', 'みて'),
    ('みなす', 'to regard', 'みなして'),
    ('まける', 'to lose (a game)', 'まけて'),
    ('やせる', 'to lose weight', 'やせて'),
    ('おとす', 'to drop', 'おとして'),   
    ('うしなう', 'to lose', 'うしなって'),
    ('なくす', 'to lose something', 'なくして'),
    ('あいする', 'to love', 'あいして'),
    ('かわいがる', 'to be affectionate', 'かわいがって'),
    ('さげる', 'to lower', 'さげて'),
    ('つくる', 'to make', 'つくって'),
    ('いちゃいちゃする', 'to flirt', 'いちゃいちゃして'),
    ('まちがえる', 'to make a mistake', 'まちがえて'),
    ('かせぐ', 'to earn', 'かせいで'),
    ('さわぐ', 'to make noise', 'さわいで'),
    ('こしらえる', 'to make, to prepare', 'こしらえて'),
    ('けっこんする', 'to marry', 'けっこんして'),
    ('あう', 'to meet', 'あって'),
    ('じゅくす', 'to mature', 'じゅくして'),
    ('はかる', 'to measure', 'はかって'),
    ('あう', 'to fit, to match', 'あって'),
    ('とける', 'to melt', 'とけて'), 
    ('どじる', 'to mess up', 'どじって'),
    ('はずす', 'to miss, to remove', 'はずして'),
    ('まぜる', 'to mix', 'まぜて'),
    ('うごく', 'to move', 'うごいて'),
    ('うごかす', 'to move (something)', 'うごかして'),
    ('とちる', 'to flub', 'とちって'),
    ('かける', 'to hang', 'かけて'),
    ('いる', 'to be, to exist', 'いて'),
    ('しらせる', 'to notify', 'しらせて'),
    ('おこる', 'to happen', 'おこって'),
    ('あける', 'to open', 'あけて'),
    ('ひらく', 'to open (more broadly)', 'ひらいて'),
    ('ちゅうもんする', 'to order', 'ちゅうもんして'),
    ('みなぎる', 'to overflow', 'みなぎって'),
    ('みのがす', 'to overlook', 'みのがして'),
    ('たおす', 'to overthrow', 'たおして'), 
    ('ぬる', 'to paint', 'ぬって'),
    ('あわてる', 'to panic', 'あわてて'),
    ('もじる', 'to parody', 'もじって'),
    ('わかれる', 'to part from', 'わかれて'),
    ('よぎる', 'to pass by', 'よぎって'),
    ('すぎる', 'to pass through', 'すぎて'),
    ('はらう', 'to pay', 'はらって'),
    ('ほろびる', 'to perish', 'ほろびて'),
    ('くどく', 'to persist', 'くどいて'),
    ('せびる', 'to pester', 'せびって'),
    ('うつす', 'to photograph', 'うつして'),
    ('ひろう', 'to pick up', 'ひろって'),
    ]
    return verbs

def quiz_game(mode, direction):
    print("Welcome to the Japanese Verb Quiz!", flush=True)
    if direction == 'te':
        print("Provide the -te form of the given verb in hiragana.", flush=True)
    elif direction == 'verb':
        print("Provide the verb corresponding to the given -te form in hiragana.", flush=True)
    else:
        print("Invalid direction selected. Please choose 'te' or 'verb'.", flush=True)
        sys.exit(1)

    correct_count = 0

    if mode == 'random':
        print("Mode: Random verbs from Jisho API.", flush=True)
    elif mode == 'predefined':
        print("Mode: Predefined list of verbs.", flush=True)
        verbs = get_predefined_verbs()
    else:
        print("Invalid mode selected. Please choose 'random' or 'predefined'.", flush=True)
        sys.exit(1)

    while True:
        verb, meaning, te_form = random.choice(verbs)

        if direction == 'te':
            print(f"\nWhat is the -te form of '{verb}' ({meaning})?", flush=True)
            answer = input("Your answer: ").strip()
            correct_answer = te_form
        elif direction == 'verb':
            print(f"\nWhat is the original verb for '{te_form}' ({meaning})?", flush=True)
            answer = input("Your answer: ").strip()
            correct_answer = verb

        if answer == correct_answer:
            correct_count += 1
            print(f"{correct_answer} is correct! You've got {correct_count} correct answers so far.", flush=True)
        else:
            print(f"Wrong. The correct answer is '{correct_answer}', not '{answer}'.", flush=True)

        # Optionally add a delay before the next question
        time.sleep(1)

if __name__ == "__main__":
    mode = 'predefined'
    direction = 'te'
    quiz_game(mode, direction)
