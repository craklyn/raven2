#include "general/secrets.h"

PlayerChore chores[] = {
    {CHORE_QUEST, 18004}, //Mayor:Kill barb warrior
    {CHORE_QUEST, 18008}, //Mayor:Kill Rendor
    {CHORE_QUEST, 18016}, //Mayor:Kill quickling
    {CHORE_QUEST, 18019}, //Mayor:Kill Macsen
    {CHORE_QUEST, 18020}, //Mayor:Kill Monkey 
    {CHORE_QUEST, 18026}, //Mayor:Kill Dervel 
    {CHORE_QUEST, 18029}, //Mayor:Kill Kuo-toan Priestess
    {CHORE_QUEST, 18034}, //Mayor:Kill Madam Zash
    {CHORE_QUEST, 18039}, //Mayor:kill follower mad
    {CHORE_QUEST, 55103}, //Primrose:yellow dress
    {CHORE_QUEST, 55104}, //Faerie:ogre feather
    {CHORE_QUEST,  7401}, //Ivan:Cheese stick
    {CHORE_QUEST,  7405}, //Ivan: wood elf
    {CHORE_QUEST,  7408}, //Ivan:whale boots
    {CHORE_QUEST,  7409}, //Ivan:Picnic Lunch
    {CHORE_QUEST,  7412}, //Jarl:Sword
    {CHORE_QUEST,  7414}, //Alchem:Lizard tongue
    {CHORE_QUEST,  7421}, //Rhyvis:slab of beef
    {CHORE_QUEST,  7422}, //Rhyvis:Seaweed
    {CHORE_QUEST,  7425}, //Rhyvis:Leafy Green
    {CHORE_QUEST, 27124}, //wizen: Kill a medusa
    {CHORE_QUEST, 27129}, //wizen: Return the shield of ancient harmony.
    {CHORE_QUEST, 27118}, //wizen: Kill Ssirak and return the tome.
    {CHORE_QUEST, 27105}, //wizen: Slay Flidalis and return her armor and head.
    {CHORE_QUEST, 27122}, //wizen: Kill Evergreen and get ring of autumn leaves
    {CHORE_QUEST, 27133}, //wizen: Kill a Tempest in Soth's Playground.
    {CHORE_QUEST, 27111}, //wizen: Slay a shady character and return his ranseur dagger.
    {CHORE_QUEST, 27113}, //wizen: Slay the minotaur god and return his claws.
    {CHORE_QUEST, 27132}, //wizen: Kill the Horrifying ghoul on the Ebony queen.
    {CHORE_QUEST, 27110}, //wizen: Slay the mother phoenix and return the egg.
    {CHORE_KILL,    903}, //River Island of Minos, Forest Minotaur
    {CHORE_KILL,   1108}, //Thieves' Den, leader of the thieves
    {CHORE_KILL,   1319}, //Three of Swords, Gabadiel
    {CHORE_KILL,   1414}, //In the Air, Kitten 
    {CHORE_KILL,   1506}, //Kingdom of Juargan, Grimdale
    {CHORE_KILL,   1504}, //Kingdom of Juargan, Juargan
    {CHORE_KILL,   1724}, //Land of Fire Newts, lumbering Neanderthal
    {CHORE_KILL,   1806}, //Haven, snow queen
    {CHORE_KILL,   1928}, //Garamond, guardian 
    {CHORE_KILL,   2003}, //Draconia, powerful mage 
    {CHORE_KILL,   2215}, //Gnomes, fire bat
    {CHORE_KILL,   2311}, //Mahn-Tor, Ogre Chief
    {CHORE_KILL,   2323}, //Mahn-Tor, Minotaur Butler
    {CHORE_KILL,   2329}, //Mahn-Tor, Sumaron
    {CHORE_KILL,   2417}, //Shire, dwarven prince
    {CHORE_KILL,   2513}, //Dragonia, golden dragon
    {CHORE_KILL,   2525}, //Dragonia, Bishop dragon
    {CHORE_KILL,   2614}, //elven valley, antelope
    {CHORE_KILL,   2813}, //Asgard, Battle Master 
    {CHORE_KILL,   2825}, //Asgard, Goddess Freya
    {CHORE_KILL,   3143}, //S Midgaard, Baron of Midgaard
    {CHORE_KILL,   3315}, //N Midgaard, Terwin
    {CHORE_KILL,   3414}, //Tunnel Maze, mad etcher  
    {CHORE_KILL,   3804}, //Isle of Tuatha, Mananan mac Lir
    {CHORE_KILL,   3903}, //Mystic Sea, Obsidian Guardian
    {CHORE_KILL,   4101}, //Moria, troll 
    {CHORE_KILL,   4317}, //castle of swords, snap-dragon
    {CHORE_KILL,   4310}, //castle of swords, third knight of sword
    {CHORE_KILL,   4660}, //Underworld, human rogue
    {CHORE_KILL,   5010}, //Eastern Desert, dracolich
    {CHORE_KILL,   5109}, //City of Drow, yochlol
    {CHORE_KILL,   5210}, //City of Thalos, Behern
    {CHORE_KILL,   5307}, //Enfans, guard dog
    {CHORE_KILL,   5511}, //Swamp of thanos, Queen of Widows
    {CHORE_KILL,   5528}, //Swamp of Thanos, Lich of Thanos
    {CHORE_KILL,   5813}, //Pyramid of Serp, Jailor
    {CHORE_KILL,   5906}, //lair of Aspis, aspis cow
    {CHORE_KILL,   6115}, //Haondor(dark), Shargugh
    {CHORE_KILL,   6248}, //Isle of Pirates, Alindra
    {CHORE_KILL,   6231}, //Isle of Pirates, Mizzenmast
    {CHORE_KILL,   6217}, //Isle of Pirates, Smallbody
    {CHORE_KILL,   6315}, //Arachnos, Ki-Rin
    {CHORE_KILL,   6502}, //Dwarven Caverns, mirror image
    {CHORE_KILL,   6604}, //Temple of time, Second Priest
    {CHORE_KILL,   7748}, //Cloudy Mountain, Bazarnog
    {CHORE_KILL,   7754}, //Cloudy Mountain, Commander Gauzrah
    {CHORE_KILL,   7710}, //Cloudy Mountain, winged dragon
    {CHORE_KILL,   7901}, //Ebony Queen, Captain Gravic 
    {CHORE_KILL,   8006}, //Soths, Warden 
    {CHORE_KILL,   8117}, //Solamnic Knights, Great Silver Wyrm
    {CHORE_KILL,   8202}, //Silver Lady, Zeigan
    {CHORE_KILL,   8301}, //Xanth, Gap Dragon
    {CHORE_KILL,   8427}, //Myth Tyr, pile of bones
    {CHORE_KILL,   8401}, //Myth Tyr, Garckin
    {CHORE_KILL,   8706}, //Urkiish Und, Magistrate
    {CHORE_KILL,   9110}, //Volcano, fiend 
    {CHORE_KILL,   9601}, //Drekken Noir, Midnight Guard
    {CHORE_KILL,   9712}, //Valley of Dawn, Emperor 
    {CHORE_KILL,   9822}, //Hamlet of Dusk, Kennel Owner
    {CHORE_KILL,  10029}, //LupusVille (Day), bellringer
    {CHORE_KILL,  10115}, //LupusVille (Night), Cat Lord
    {CHORE_KILL,  11207}, //Valley Mad Mage, Troll Mage
    {CHORE_KILL,  11506}, //Shaden, Baron
    {CHORE_KILL,  12108}, //Irridenta, mama Griffon 
    {CHORE_KILL,  12308}, //Duke Arilan, Jeeves 
    {CHORE_KILL,  12501}, //Freehold Thieves, Honorable Man
    {CHORE_KILL,  12609}, //Blackhand, witch 
    {CHORE_KILL,  12719}, //Michaeline Keep, Sephriana 
    {CHORE_KILL,  12805}, //Dragonor Keep, Flower 
    {CHORE_KILL,  13049}, //HighTower, Necromancer
    {CHORE_KILL,  13280}, //Astral Ext, githyanki warlock
    {CHORE_KILL,  13529}, //New Thalos, Aziz
    {CHORE_KILL,  13806}, //Olympus, Ares
    {CHORE_KILL,  13916}, //Warcamp, Baltac
    {CHORE_KILL,  14002}, //Trial of the Initiate, Ludwig
    {CHORE_KILL,  15106}, //Dark Friends, Guild Clerk 
    {CHORE_KILL,  15257}, //Dreamworld, Medusa 
    {CHORE_KILL,  15323}, //Lord Vrolock, Carnifern
    {CHORE_KILL,  15336}, //Lord Vrolock, Lizard Man
    {CHORE_KILL,  15421}, //BlackMarket, L'orin
    {CHORE_KILL,  15604}, //Tombs o Tarin, Silver Dragon
    {CHORE_KILL,  15722}, //Legend, yellowjacket
    {CHORE_KILL,  16103}, //Isle of Myst, lightening
    {CHORE_KILL,  16215}, //Isle of Myst, shrieker 
    {CHORE_KILL,  17015}, //Deep Moria, mine leader 
    {CHORE_KILL,  17207}, //Farm, Scarecrow
    {CHORE_KILL,  17417}, //Orchard, faerie warrior
    {CHORE_KILL,  18335}, //Knights of Krynn, Vampiress
    {CHORE_KILL,  18398}, //Knights of Krynn, Dark Daughter
    {CHORE_KILL,  18804}, //Dwarven Village, Daphne
    {CHORE_KILL,  18902}, //Lost Temple, Statue 
    {CHORE_KILL,  19029}, //Emerald Forest, marmalade cat
    {CHORE_KILL,  19109}, //Elemental Canyon, Fire Ruler
    {CHORE_KILL,  19203}, //Rats Lair, Sammy
    {CHORE_KILL,  20216}, //Blackheart Keep, Karik
    {CHORE_KILL,  20507}, //Kenku Village, Kenku King
    {CHORE_KILL,  20605}, //Under Dark, Huge Beholder
    {CHORE_KILL,  20672}, //Under Dark, Shillikif
    {CHORE_KILL,  20699}, //Under Dark, Ass Guildmaster
    {CHORE_KILL,  20710}, //Under Dark, Lord Rygar
    {CHORE_KILL,  21223}, //Forgotten City, Balor
    {CHORE_KILL,  21324}, //Forgotten Forest, Ariel
    {CHORE_KILL,  21521}, //UnderDark Ext, Vampire Mage
    {CHORE_KILL,  21840}, //Siege on Malfor's, Black Demon
    {CHORE_KILL,  22502}, //Portals of time, Keeper Fire
    {CHORE_KILL,  22932}, //Freeport, Lord Mourning
    {CHORE_KILL,  23002}, //Perceptory, Caritas
    {CHORE_KILL,  23418}, //New Camelot, Mordred
    {CHORE_KILL,  23445}, //New Camelot, Saxon Chief
    {CHORE_KILL,  23500}, //Ocean, Kraken 
    {CHORE_KILL,  23662}, //Dead Man's, Keclutrac
    {CHORE_KILL,  23805}, //Plains Shierre, peacock 
    {CHORE_KILL,  24033}, //Les Foret, Chopper 
    {CHORE_KILL,  24716}, //Usurpers, Badwick
    {CHORE_KILL,  27122}, //Meth-Mertul, Terelat Officer
    {CHORE_KILL,  27201}, //Land of Jermlaine, Jerm Brat
    {CHORE_KILL,  27600}, //Everwar, Unger Troll 
    {CHORE_KILL,  30408}, //Fairhaven, Warden 
    {CHORE_KILL,  30806}, //Fair Sewers, Crayfish 
    {CHORE_KILL,  31027}, //DT Dungeon, elven thief
    {CHORE_KILL,  31307}, //Rabbit Hole, Mother rabbit
    {CHORE_KILL,  31811}, //Blight, Aes Sedai 
    {CHORE_KILL,  32306}, //Village Turra, Gnome leader
    {CHORE_KILL,  32416}, //Dungeons Denegorth, Monk Amal
    {CHORE_KILL,  32605}, //Forest Turra, elven chief
    {CHORE_KILL,  33006}, //River Dangers, tiny alligator
    {CHORE_KILL,  34007}, //Xak Tsaroth, Phudge
    {CHORE_KILL,  34110}, //Crystal Palace, Kallist
    {CHORE_KILL,  39730}, //Atlantis, Leviathan
    {CHORE_KILL,  39887}, //drakyri Isle, frail man
    {CHORE_KILL,  40122}, //Steeding, Carn ape 
    {CHORE_KILL,  43406}, //SKG, lich 
};

int num_chores = sizeof(chores) / sizeof(PlayerChore);
