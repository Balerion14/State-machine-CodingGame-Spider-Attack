#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include<array>
#include <cmath>
#include <tuple>
#include <ctime>


using namespace std;

// Classe gerer valeur generale base
class Base
{
    struct values_Base
    {
        int base_x; // le coin de la map represente la base allie
        int base_y;
        int health;
        int mana; // Not used in league...
        int mana_old = 0;
    };

    public:

        std::array<values_Base, 2> List_Base; // Tableau contenant les valeurs generales des 2 bases, 0 = allie, ennemie = 1

        int heroes_per_player; // toujours  = 3

    public:

        Base() = default;

        void init_values_1_base()
        {
            cin >> List_Base[0].base_x >> List_Base[0].base_y; cin.ignore(); // Allie
            deduction_coor_base_ennemie(List_Base[0].base_x, List_Base[0].base_y); // Determiner ou se trouve base ennemie...
            cin >> heroes_per_player; cin.ignore(); // 3
        }

        void init_values_2_base()
        { 
            for (int i = 0; i < 2; i++)
            {
                if(i == 0){List_Base[0].mana_old = List_Base[0].mana ;cin >> List_Base[0].health >> List_Base[0].mana; cin.ignore();} // Allie
                else{List_Base[1].mana_old = List_Base[1].mana ;cin >> List_Base[1].health >> List_Base[1].mana; cin.ignore();} // Ennemie  
            }
        }

        void deduction_coor_base_ennemie(int base_x, int base_y)
        {
            if(base_x == 0 && base_y == 0){List_Base[1].base_x = 17630; List_Base[1].base_y = 9000;}
            else{List_Base[1].base_x = 0; List_Base[1].base_y = 0;}
        }
};

class entity
{
    struct hero_Allie
    {
        int id; // Unique identifier
        int x; // Position of this entity
        int y;
        int shield_life; // Ignore for this league; Count down until shield spell fades
        int is_controlled; // Ignore for this league; Equals 1 when this entity is under a control spell
    };

    struct hero_Ennemie
    {
        int id; // Unique identifier
        int x; // Position of this entity
        int y;
        int shield_life; // Ignore for this league; Count down until shield spell fades
        int is_controlled; // Ignore for this league; Equals 1 when this entity is under a control spell
        float level_danger = 0; // Niveau danger
    };

    struct monstre
    {
        int id; // Unique identifier
        int x; // Position of this entity
        int y;
        int shield_life; // Ignore for this league; Count down until shield spell fades
        int is_controlled; // Ignore for this league; Equals 1 when this entity is under a control spell
        int health; // Remaining health of this monster
        int vx; // Trajectory of this monster
        int vy;
        int near_base; // 0=monster with no target yet, 1=monster targeting a base
        int threat_for; // Given this monster's trajectory, is it a threat to 1=your base, 2=your opponent's base, 0=neither
        float level_danger = 0; //Niveau danger
    };

    public:

        vector<hero_Allie> List_hero_Allie;
        vector<hero_Ennemie> List_hero_Ennemie;
        vector<monstre> List_monstre;

        int entity_count; // Amount of heros and monsters you can see

    public:

        entity() = default;

        void init_value_entity()
        {
            cin >> entity_count; cin.ignore();
            for (int i = 0; i < entity_count; i++)
            {
                int id; // Unique identifier
                int type; // 0=monster, 1=your hero, 2=opponent hero
                int x; // Position of this entity
                int y;
                int shield_life; // Ignore for this league; Count down until shield spell fades
                int is_controlled; // Ignore for this league; Equals 1 when this entity is under a control spell
                int health; // Remaining health of this monster//
                int vx; // Trajectory of this monster
                int vy;
                int near_base; // 0=monster with no target yet, 1=monster targeting a base
                int threat_for; // Given this monster's trajectory, is it a threat to 1=your base, 2=your opponent's base, 0=neither

                cin >> id >> type >> x >> y >> shield_life >> is_controlled >> health >> vx >> vy >> near_base >> threat_for; cin.ignore();

                if(type == 1){hero_Allie hero = {id, x, y, shield_life, is_controlled}; List_hero_Allie.push_back(hero);} // hero
                else if(type == 2){hero_Ennemie hero = {id, x, y, shield_life, is_controlled}; List_hero_Ennemie.push_back(hero);} // opposent_hero
                else{monstre monstre = {id, x, y, shield_life, is_controlled, health, vx, vy, near_base, threat_for}; List_monstre.push_back(monstre);} // monstre
            }
        }

        void clear_array()
        {
            List_hero_Allie.clear();
            List_hero_Ennemie.clear();
            List_monstre.clear();
        }
};

class test
{
    public:
    public:

    test() = default;
};

class IA
{
    public:

        Base base;
        entity entite;
        test test_1;

        double debut;
        int save_index_0;
        int save_index_1;
        int save_index_2;
        int save_index_3;
        bool activ;
        bool activ_2;

        const int ZONE_1_max_A = 5000; // En partant de ma base
        const int ZONE_2_max_A = 9000;
        const int ZONE_3_max_E = 9000; // En partant de la base ennemie
        
        //Tableau pour calculer distance Hero -> Monstre, hero, hero opposent [j][i] j == nombre de heros allie(3), i == nombre tous les element pour chaque heros
        vector<vector<tuple<int,double>>> list_id_dist_hero_M; // Tableau contenant la distance et l index(case array origine) hero -> monstre
        vector<vector<tuple<int,double>>> list_id_dist_hero_H; // Tableau contenant la distance et l index(case array origine) hero -> hero
        vector<vector<tuple<int,double>>> list_id_dist_hero_HO; // Tableau contenant la distance et l index(case array origine) hero -> hero_opposent

        //Tableau pour calculer distance Base -> toutes les entites...(Attention l id retourner == id dans mes tableau de strcu lors de linitialisation pour eviter de refaire recherche pour voir a quelle case c est)
        vector<tuple<int,double>> list_id_dist_Base_allie_monstre; // dist entre base allie et monstre // id, distance
        vector<tuple<int,double>> list_id_dist_Base_ennemie_monstre; // dist entre base ennemie et monstre // id, distance
        vector<tuple<int,double>> list_id_dist_Base_allie_hero; // id case + distance entre base allie et hero(tous), hero allie(0,1,2) et apres opposent_hero
        vector<tuple<int,double>> list_id_dist_Base_ennemie_hero; //idem qu'en haut mais cette  fois de la base ennemie vers hero(tous) 

        //Tableau pour stocker situation zone
        vector<tuple<int,int>> list_zone_1;//type et index dans tableau(0 monstre et 2 hero opposent)
        vector<tuple<int,int>> list_zone_2;
        vector<tuple<int,int>> list_zone_3;

        //Tableau monstre viseront base au prochain coup
        vector<int> aligne;

        //Tableau qui sotcke id et distance monstre par rapport a chaque hero opposent
        vector<vector<tuple<int,double>>> list_id_distance_hero_opposent_monstre; // [0]->hero ennemie 3, [1]->hero ennemie 4, [2]->hero ennemie 5,//Ca correspond au case de mon tableau de base car j'ai fait en sorte que ca soit proportionnelle(tb_base[0]=new tableau[0])
        
        //tableau qui repertorie les meilleurs " sort Controle" possible pour chaque hero (hero[0] defense->[0] best monstre et [1] best opposent, attack->[0] best monstre et [1] best opposent et si rien alors -1 dans les champs )
        vector<vector<tuple<int,int,int,int>>> coord_best_Controle; // contiendrait mes 3 heros et pour chaque hero(x, y, type, index) et different coup selon la situation voulu(defensive, offensive)
        
        //Tableau contenant cible des wind pour chaque hero
        vector<vector<tuple<bool,int,int,int>>> autorisation_wind; // autorisatrion de wind et l'index du hero correspondant([0][p]->premier case( wind ou pas et hero index) et les autre case sur ce meme hero(type(0 monstre, 1 hero opposent) et l autre case  l index de l entity reperer)), ciblage x et y
        
        //Tableau sort urgent
        vector<tuple<std::string,int,int,int,int>> choose_sort;// nom sort, x et y direction finale, entite vise(-1 si c est un wind(on donne la position dans tableau et pas directement l id)) et l'index hero qui fait le sort

        //Tableau de best shiel
        vector<tuple<int,int,int>> best_shield;// id cible, type, tireur(index tableau)hero 0, 1, 2

        //Tableau best move pour chaque hero en fonctio ndes situation
        vector<tuple<int,int,int>> best_deplacement; // index tableau hero(pas l id),x, y

        //Tableau des meilleurs mouvements pour wait(3 heros)
        int coord [3][10] = { {3047,6908, 5482,8200, 11000,1265, 8300,1539, 3047,6908}, {8303,1812, 3047,6908, 4526,7500, 9873,1789, 8303,1812}, {12012,7819, 13878,5065, 16813,4315, 15722,6954, 12012,7819}};
        int coord_2 [3][10] = { {9259,7600, 7825,7917, 11762,2130, 14600,3313, 9259,7600}, {13673,2635, 10669,7103, 8417,7364, 12171,2039, 13673,2635}, {5550,1243, 3500,3700, 658,4952, 794,2904, 1386,1470}};
        int coord_3 [2][10] = { {4094,5400, 8394,3700, 10283,1293, 8300,1038, 4094,5400}, {9395,6294, 10965,3837, 12308,1743, 10419,970, 9395,6294}};

    public:

        IA() : base(), entite(), test_1()
        {cerr << "Initialisation du jeu" << endl;}

        void loop_game()
        {
            save_index_0 = 0;
            save_index_1 = 0;
            save_index_2 = 0;
            save_index_3 = 0;
            activ = false;
            activ_2 = false;
            std::clock_t c_start = std::clock();

            debut = c_start;
            base.init_values_1_base();

            while(1)
            {
                base.init_values_2_base();
                entite.init_value_entity();
                Best_action();

                //clear
                _clear();
            }
        }

        void _clear()
        {
            entite.clear_array();
            list_id_dist_hero_M.clear();
            list_id_dist_hero_H.clear();
            list_id_dist_hero_HO.clear();

            list_id_dist_Base_allie_monstre.clear();
            list_id_dist_Base_ennemie_monstre.clear();
            list_id_dist_Base_allie_hero.clear();
            list_id_dist_Base_ennemie_hero.clear();

            list_zone_1.clear();
            list_zone_2.clear();
            list_zone_3.clear();

            aligne.clear();

            list_id_distance_hero_opposent_monstre.clear();

            coord_best_Controle.clear();

            autorisation_wind.clear();

            choose_sort.clear();

            best_shield.clear();

            best_deplacement.clear();
        }

        void Best_action()
        {
            calcule_distance_monstre_base(entite.List_monstre.size());
            calcule_distance_hero_base(entite.List_hero_Allie.size());
            distance_hero_entity();
            verif_Etat_zone();
            id_dist_heroopposent_monstre();

            cerr << "----------Eval situation monstre" << endl;
            evaluation_situation_monstre_hero_opposent_visible();
            choose_best_Constrol_hero();
            choose_best_wind();
            _best_shield();
            choose_best_move();
            best_commande();
        }

        void best_commande()
        {
            int mana = base.List_Base[0].mana;
            if(mana > 250){activ = true;}
            int indexsave = 0;
            for(int m = 0; m <entite.List_monstre.size();m++){if(entite.List_monstre[m].health >=19){activ_2 = true;}}
            for(int o = 0; o < 3; o++)
            {
                double distance_hero = 0;
                for(int q = 0; q < list_id_dist_Base_allie_hero.size(); q++)
                {
                    if(o == get<0>(list_id_dist_Base_allie_hero[q])){distance_hero = get<1>(list_id_dist_Base_allie_hero[q]);}
                    if(q == 2){q = list_id_dist_Base_allie_hero.size();}
                }

                //Recup possibilite sort controle
                int index_sort_CONTROl_D_M = -1;
                int index_2_sort_CONTROl_D_HO = -1;
                int index_3_sort_CONTROl_OFF_M = -1;
                int index_4_sort_CONTROl_OFF_HO = -1;

                for(auto i = 0 ; i<coord_best_Controle.size(); i++)
                {
                    if(i == o)
                    {
                        for(int p = 0; p<coord_best_Controle[i].size(); p++)
                        {
                            if(p == 0 && get<3>(coord_best_Controle[i][p]) != -1)
                            {index_sort_CONTROl_D_M = p;}
                            else if(p == 1 && get<3>(coord_best_Controle[i][p]) != -1)
                            {index_2_sort_CONTROl_D_HO = p;}
                            else if(p == 2 && get<3>(coord_best_Controle[i][p]) != -1)
                            {index_3_sort_CONTROl_OFF_M = p;}
                            else if(p == 3 && get<3>(coord_best_Controle[i][p]) != -1)
                            {index_4_sort_CONTROl_OFF_HO = p;}
                        }
                    } 
                }
                if(activ == true)
                {
                    if(mana >=8 && get<2>(best_shield[o]) !=-1){mana = -8;cout << "SPELL" << " " << "SHIELD" << " " << entite.List_monstre[get<2>(best_shield[o])].id << " " << "Shield" << " " << endl;}
                    else if(mana >= 16 && get<0>(autorisation_wind[o][0]) != 0){mana = -8; ;cout << "SPELL" << " " << "WIND"  << " "<< get<2>(autorisation_wind[o][0]) << " " << get<3>(autorisation_wind[o][0]) << " " << "Wind" << endl;}
                    else if(mana >= 16 && (index_sort_CONTROl_D_M != -1 || index_2_sort_CONTROl_D_HO != -1 || index_3_sort_CONTROl_OFF_M != -1 || index_4_sort_CONTROl_OFF_HO != -1))
                    {
                        mana = -8;
                        if(index_2_sort_CONTROl_D_HO != -1){cout << "SPELL" << " " << "CONTROL" << " " << entite.List_hero_Ennemie[get<3>(coord_best_Controle[o][index_2_sort_CONTROl_D_HO])].id << " " << get<0>(coord_best_Controle[o][index_2_sort_CONTROl_D_HO]) << " " << get<1>(coord_best_Controle[o][index_2_sort_CONTROl_D_HO]) << " " << "Control" << endl;}
                        else if(index_sort_CONTROl_D_M != -1){cout << "SPELL" << " " << "CONTROL" << " " << entite.List_monstre[get<3>(coord_best_Controle[o][index_sort_CONTROl_D_M])].id << " " << get<0>(coord_best_Controle[o][index_sort_CONTROl_D_M]) << " " << get<1>(coord_best_Controle[o][index_sort_CONTROl_D_M]) << " " << "Control" << endl;}
                        else if(index_4_sort_CONTROl_OFF_HO != -1){cout << "SPELL" << " " << "CONTROL" << " " << entite.List_hero_Ennemie[get<3>(coord_best_Controle[o][index_4_sort_CONTROl_OFF_HO])].id << " " << get<0>(coord_best_Controle[o][index_4_sort_CONTROl_OFF_HO]) << " " << get<1>(coord_best_Controle[o][index_4_sort_CONTROl_OFF_HO]) << " " << "Control" <<  endl;}
                        else if(index_3_sort_CONTROl_OFF_M != -1){cout << "SPELL" << " " << "CONTROL" << " " << entite.List_monstre[get<3>(coord_best_Controle[o][index_3_sort_CONTROl_OFF_M])].id << " " << get<0>(coord_best_Controle[o][index_3_sort_CONTROl_OFF_M]) << " " << get<1>(coord_best_Controle[o][index_3_sort_CONTROl_OFF_M]) << " " << "Control" << endl;}
                    }
                    else if(get<1>(best_deplacement[o]) != -1 && get<2>(best_deplacement[o]) != -1){cout << "MOVE" << " " << get<1>(best_deplacement[o]) << " " << get<2>(best_deplacement[o]) << " " << "Move" << endl;}
                    else
                    {
                        if(save_index_0 == 10){save_index_0 = 0;}
                        if(save_index_1 == 10){save_index_1 = 0;}
                        if(save_index_2 == 10){save_index_2 = 0;}
                        if(entite.List_hero_Allie[0].x == coord[0][save_index_0] && entite.List_hero_Allie[0].y == coord[0][save_index_0 + 1]){save_index_0 += 2;}
                        if(entite.List_hero_Allie[1].x == coord[1][save_index_1] && entite.List_hero_Allie[1].y == coord[1][save_index_1 + 1]){save_index_1 += 2;}
                        if(entite.List_hero_Allie[2].x == coord[2][save_index_2] && entite.List_hero_Allie[2].y == coord[2][save_index_2 + 1]){save_index_2 += 2;}
                        if(base.List_Base[0].base_x == 0)
                        {
                            if(o==0){cout << "MOVE" << " " << coord[o][save_index_0] << " " << coord[o][save_index_0 + 1] << endl;}
                            else if(o==1){cout << "MOVE" << " " << coord[o][save_index_1] << " " << coord[o][save_index_1 + 1] << endl;}
                            else{cout << "MOVE" << " " << coord[o][save_index_2] << " " << coord[o][save_index_2 + 1] << endl;}
                        }
                        else
                        {
                            if(entite.List_hero_Allie[0].x == coord_2[0][save_index_0] && entite.List_hero_Allie[0].y == coord_2[0][save_index_0 + 1]){save_index_0 += 2;}
                            if(entite.List_hero_Allie[1].x == coord_2[1][save_index_1] && entite.List_hero_Allie[1].y == coord_2[1][save_index_1 + 1]){save_index_1 += 2;}
                            if(entite.List_hero_Allie[2].x == coord_2[2][save_index_2] && entite.List_hero_Allie[2].y == coord_2[2][save_index_2 + 1]){save_index_2 += 2;}
                            if(o==0){cout << "MOVE" << " " << coord_2[o][save_index_0] << " " << coord_2[o][save_index_0 + 1] << endl;}
                            else if(o==1){cout << "MOVE" << " " << coord_2[o][save_index_1] << " " << coord_2[o][save_index_1 + 1] << endl;}
                            else{cout << "MOVE" << " " << coord_2[o][save_index_2] << " " << coord_2[o][save_index_2 + 1] << endl;}
                        }
                    }
                }
                else
                {
                    if(get<1>(best_deplacement[o]) != -1 && get<2>(best_deplacement[o]) != -1){
                    int index = 0;
                    for(int u = 0; u < list_id_dist_Base_allie_monstre.size(); u++)
                    {
                        if(entite.List_monstre[get<0>(list_id_dist_Base_allie_monstre[u])].x == get<1>(best_deplacement[o]) && entite.List_monstre[get<0>(list_id_dist_Base_allie_monstre[u])].y ==  get<2>(best_deplacement[o]))
                        {index = get<0>(list_id_dist_Base_allie_monstre[u]);}
                    }
                    bool urt = determinate_if_the_time(index, o);
                    sort_urgent_def_monstre(index, o);
                    if(urt == false && mana >=8 &&  choose_sort.size() > 0 && indexsave !=  entite.List_monstre[index].id && (entite.List_monstre[index].near_base == 0 && entite.List_monstre[index].threat_for == 1 || entite.List_monstre[index].near_base == 1 && entite.List_monstre[index].threat_for == 1))
                    {
                        if(get<0>(choose_sort[0]) == "WIND"){cout << "SPELL" << " " << "WIND"  << " "<< get<1>(choose_sort[0]) << " " << get<2>(choose_sort[0]) << " " << "Wind" << endl;}
                        else{cout << "SPELL" << " " << "CONTROL" << " " << entite.List_monstre[get<3>(choose_sort[0])].id << " " << get<1>(choose_sort[0]) << " " << get<2>(choose_sort[0]) << " " << "Control" << endl;}
                        indexsave = entite.List_monstre[index].id;
                        mana = -8;
                    }
                    else{cout << "MOVE" << " " << get<1>(best_deplacement[o]) << " " << get<2>(best_deplacement[o]) << " " << "Move" << endl;}
                    }
                    else
                    {
                        if(save_index_0 == 10){save_index_0 = 0;}
                        if(save_index_1 == 10){save_index_1 = 0;}
                        if(save_index_2 == 10){save_index_2 = 0;}
                        if(entite.List_hero_Allie[0].x == coord[0][save_index_0] && entite.List_hero_Allie[0].y == coord[0][save_index_0 + 1]){save_index_0 += 2;}
                        if(entite.List_hero_Allie[1].x == coord[1][save_index_1] && entite.List_hero_Allie[1].y == coord[1][save_index_1 + 1]){save_index_1 += 2;}
                        if(entite.List_hero_Allie[2].x == coord[2][save_index_2] && entite.List_hero_Allie[2].y == coord[2][save_index_2 + 1]){save_index_2 += 2;}
                        if(base.List_Base[0].base_x == 0)
                        {
                            if(o==0){cout << "MOVE" << " " << coord[o][save_index_0] << " " << coord[o][save_index_0 + 1] << endl;}
                            else if(o==1){cout << "MOVE" << " " << coord[o][save_index_1] << " " << coord[o][save_index_1 + 1] << endl;}
                            else
                            {
                                if(activ_2 == true){cout << "MOVE" << " " << coord[o][save_index_2] << " " << coord[o][save_index_2 + 1] << endl;}
                                else
                                {
                                    if(save_index_3 == 10){save_index_3 = 0;}
                                    if(entite.List_hero_Allie[2].x == coord_3[0][save_index_3] && entite.List_hero_Allie[2].y == coord_3[0][save_index_3 + 1]){save_index_3 += 2;}
                                    cout << "MOVE" << " " << coord_3[0][save_index_3] << " " << coord_3[0][save_index_3 + 1] << endl;
                                }
                            }
                        }
                        else
                        {
                            if(entite.List_hero_Allie[0].x == coord_2[0][save_index_0] && entite.List_hero_Allie[0].y == coord_2[0][save_index_0 + 1]){save_index_0 += 2;}
                            if(entite.List_hero_Allie[1].x == coord_2[1][save_index_1] && entite.List_hero_Allie[1].y == coord_2[1][save_index_1 + 1]){save_index_1 += 2;}
                            if(entite.List_hero_Allie[2].x == coord_2[2][save_index_2] && entite.List_hero_Allie[2].y == coord_2[2][save_index_2 + 1]){save_index_2 += 2;}
                            if(o==0){cout << "MOVE" << " " << coord_2[o][save_index_0] << " " << coord_2[o][save_index_0 + 1] << endl;}
                            else if(o==1){cout << "MOVE" << " " << coord_2[o][save_index_1] << " " << coord_2[o][save_index_1 + 1] << endl;}
                            else
                            {
                                if(activ_2==true){cout << "MOVE" << " " << coord_2[o][save_index_2] << " " << coord_2[o][save_index_2 + 1] << endl;}
                                else
                                {
                                    if(save_index_3 == 10){save_index_3 = 0;}
                                    if(entite.List_hero_Allie[2].x == coord_3[1][save_index_3] && entite.List_hero_Allie[2].y == coord_3[1][save_index_3 + 1]){save_index_3 += 2;}
                                    cout << "MOVE" << " " << coord_3[1][save_index_3] << " " << coord_3[1][save_index_3 + 1] << endl;
                                }
                            }
                        }
                        
                    }

                }
                
            }
        }

        void choose_best_move()
        {
            int save_id = -1;

            for(int i = 0; i < 3; i++)
            {
                double distance_hero = 0;
                int compteur_index_danger = 0;
                vector<tuple<int,double>> Save_id;//id, distance

                for(int q = 0; q < list_id_dist_Base_allie_hero.size(); q++)
                {
                    if(i == get<0>(list_id_dist_Base_allie_hero[q])){distance_hero = get<1>(list_id_dist_Base_allie_hero[q]);}
                    if(q == 2){q = list_id_dist_Base_allie_hero.size();}
                }

                if(i < 2)
                {
                    for(int q = 0; q < list_id_dist_Base_allie_monstre.size(); q++)
                    {
                        if(get<1>(list_id_dist_Base_allie_monstre[q]) < 6000){compteur_index_danger++; Save_id.push_back(make_tuple(get<0>(list_id_dist_Base_allie_monstre[q]), get<1>(list_id_dist_Base_allie_monstre[q])));}
                    }

                    if( i == 0 && compteur_index_danger > 0)
                    {
                        float max = 0;
                        int save = -1;
                        double distance_save = 0;
                        
                        for(int j = 0; j < Save_id.size(); j++) // Parcours monstre autour de chaque heros
                        {
                            if(entite.List_monstre[get<0>(Save_id[j])].level_danger > max)
                            {
                                max = entite.List_monstre[get<0>(Save_id[j])].level_danger;
                                save = get<0>(Save_id[j]);
                                distance_save = get<1>(Save_id[j]);
                            }
                            else if(entite.List_monstre[get<0>(Save_id[j])].level_danger == max)
                            {
                                if(get<1>(Save_id[j]) < distance_save)
                                {
                                    max = entite.List_monstre[get<0>(Save_id[j])].level_danger;
                                    save = get<0>(Save_id[j]);
                                    distance_save = get<1>(Save_id[j]);
                                }
                            }
                        }

                        //Remplir tableau
                        if(save != -1){best_deplacement.push_back(make_tuple(i, entite.List_monstre[save].x, entite.List_monstre[save].y));}
                        else{best_deplacement.push_back(make_tuple(i, -1, -1));}

                        //Verifier si j ai le temps de tuer ou pas
                        if(save != -1)
                        {
                            bool time = determinate_if_the_time(save, i);
                            cerr << "bool :" << time << endl;
                            if(time == true){save_id = save;}
                            else{save_id = -1;}
                        }
                    }
                    else if( i == 1 && compteur_index_danger > 0)
                    {

                        if(save_id == -1) // Besoin aide
                        {   
                            float max = 0;
                            int save = -1;
                            double distance_save = 0;

                            // monstre le plus proche et le plus dangereux autour de chaque hero
                            for(int j = 0; j < Save_id.size(); j++) // Parcours monstre autour de chaque heros
                            {
                                if(entite.List_monstre[get<0>(Save_id[j])].level_danger > max)
                                {
                                    max = entite.List_monstre[get<0>(Save_id[j])].level_danger;
                                    save = get<0>(Save_id[j]);
                                    distance_save = get<1>(Save_id[j]);
                                }
                                else if(entite.List_monstre[get<0>(Save_id[j])].level_danger == max)
                                {
                                    if(get<1>(Save_id[j]) < distance_save)
                                    {
                                        max = entite.List_monstre[get<0>(Save_id[j])].level_danger;
                                        save = get<0>(Save_id[j]);
                                        distance_save = get<1>(Save_id[j]);
                                    }
                                }  
                            }

                            //Remplir tableau
                            if(save != -1){best_deplacement.push_back(make_tuple(i, entite.List_monstre[save].x, entite.List_monstre[save].y));}
                            else{best_deplacement.push_back(make_tuple(i, -1, -1));}  
                        }
                        else // Pas besoin aide
                        {
                            float max = 0;
                            int save = -1;
                            double distance_save = 0;

                            // monstre le plus proche et le plus dangereux autour de chaque hero
                            for(int j = 0; j < Save_id.size(); j++) // Parcours monstre autour de chaque heros
                            {
                                if(save_id != get<0>(Save_id[j]) && entite.List_monstre[get<0>(Save_id[j])].level_danger > max)
                                {
                                    max = entite.List_monstre[get<0>(Save_id[j])].level_danger;
                                    save = get<0>(Save_id[j]);
                                    distance_save = get<1>(Save_id[j]);
                                }
                                else if(save_id != get<0>(Save_id[j]) && entite.List_monstre[get<0>(Save_id[j])].level_danger == max)
                                {
                                    if(get<1>(Save_id[j]) < distance_save)
                                    {
                                        max = entite.List_monstre[get<0>(Save_id[j])].level_danger;
                                        save = get<0>(Save_id[j]);
                                        distance_save = get<1>(Save_id[j]);
                                    }
                                }  
                            }

                            if(save == -1)
                            {
                                float max = 0;
                                int _save = -1;// -1 si pas de monstre
                                double distance_save = 0;
                                // monstre le plus proche et le plus dangereux autour de chaque hero
                                for(int j = 0; j < list_id_dist_hero_M.size(); j++) // Parcours monstre autour de chaque heros
                                {
                                    if(j == i)
                                    {
                                        for(int e = 0; e < list_id_dist_hero_M[j].size(); e++)
                                        {
                                            if(get<1>(list_id_dist_hero_M[j][e]) < 2200 && entite.List_monstre[get<0>(list_id_dist_hero_M[j][e])].level_danger > max)
                                            {
                                                max = entite.List_monstre[get<0>(list_id_dist_hero_M[j][e])].level_danger;
                                                _save = get<0>(list_id_dist_hero_M[j][e]);
                                                distance_save = get<1>(list_id_dist_hero_M[j][e]);
                                            }
                                            else if(get<1>(list_id_dist_hero_M[j][e]) < 2200 && entite.List_monstre[get<0>(list_id_dist_hero_M[j][e])].level_danger == max)
                                            {
                                                if(get<1>(list_id_dist_hero_M[j][e]) < distance_save)
                                                {
                                                    max = entite.List_monstre[get<0>(list_id_dist_hero_M[j][e])].level_danger;
                                                    _save = get<0>(list_id_dist_hero_M[j][e]);
                                                    distance_save = get<1>(list_id_dist_hero_M[j][e]);
                                                }
                                            }
                                        } 
                                    }
                                }

                                //Remplir tableau
                                if(_save != -1){best_deplacement.push_back(make_tuple(i, entite.List_monstre[_save].x, entite.List_monstre[_save].y));}
                                else{best_deplacement.push_back(make_tuple(i, -1, -1));}
                            }
                            else
                            {
                                //Remplir tableau
                                if(save != -1){best_deplacement.push_back(make_tuple(i, entite.List_monstre[save].x, entite.List_monstre[save].y));}
                                else{best_deplacement.push_back(make_tuple(i, -1, -1));}  
                            }
                        }
                    }
                    else
                    {
                        float max = 0;
                        int save = -1;// -1 si pas de monstre
                        double distance_save = 0;
                        // monstre le plus proche et le plus dangereux autour de chaque hero
                        for(int j = 0; j < list_id_dist_hero_M.size(); j++) // Parcours monstre autour de chaque heros
                        {
                            if(j == i)
                            {
                                for(int e = 0; e < list_id_dist_hero_M[j].size(); e++)
                                {
                                    if(get<1>(list_id_dist_hero_M[j][e]) < 2200 && entite.List_monstre[get<0>(list_id_dist_hero_M[j][e])].level_danger > max)
                                    {
                                        max = entite.List_monstre[get<0>(list_id_dist_hero_M[j][e])].level_danger;
                                        save = get<0>(list_id_dist_hero_M[j][e]);
                                        distance_save = get<1>(list_id_dist_hero_M[j][e]);
                                    }
                                    else if(get<1>(list_id_dist_hero_M[j][e]) < 2200 && entite.List_monstre[get<0>(list_id_dist_hero_M[j][e])].level_danger == max)
                                    {
                                        if(get<1>(list_id_dist_hero_M[j][e]) < distance_save)
                                        {
                                            max = entite.List_monstre[get<0>(list_id_dist_hero_M[j][e])].level_danger;
                                            save = get<0>(list_id_dist_hero_M[j][e]);
                                            distance_save = get<1>(list_id_dist_hero_M[j][e]);
                                        }
                                    }
                                } 
                            }
                        }

                        //Remplir tableau
                        if(save != -1){best_deplacement.push_back(make_tuple(i, entite.List_monstre[save].x, entite.List_monstre[save].y));}
                        else{best_deplacement.push_back(make_tuple(i, -1, -1));}  
                    }
                }
                else // Celui qui sera en attaque va prendre tous les monstres le moins dangereux pour lui qui sont les plus proches(nuance ce monstre se trouvera dans la zone ennemie)
                {
                    if(distance_hero < 12000)
                    {
                        int save = -1;
                        double distance_save = 100000;
                        // monstre le plus proche et le plus dangereux autour de chaque hero
                        for(int j = 0; j < list_id_dist_hero_M.size(); j++) // Parcours monstre autour de chaque heros
                        {
                            if(j == i)
                            {
                                for(int e = 0; e < list_id_dist_hero_M[j].size(); e++)
                                {
                                    if(get<1>(list_id_dist_hero_M[j][e]) < 2200 && (entite.List_monstre[get<0>(list_id_dist_hero_M[j][e])].near_base == 1 && entite.List_monstre[get<0>(list_id_dist_hero_M[j][e])].threat_for == 1 || entite.List_monstre[get<0>(list_id_dist_hero_M[j][e])].near_base == 0 && (entite.List_monstre[get<0>(list_id_dist_hero_M[j][e])].threat_for == 0 || entite.List_monstre[get<0>(list_id_dist_hero_M[j][e])].threat_for == 1)))
                                    {
                                        if(get<1>(list_id_dist_hero_M[j][e]) < distance_save)
                                        {
                                            distance_save = get<1>(list_id_dist_hero_M[j][e]);
                                            save = get<0>(list_id_dist_hero_M[j][e]);
                                        }
                                    }
                                } 
                            }
                        }

                        //Remplir tableau
                        if(save != -1){best_deplacement.push_back(make_tuple(i, entite.List_monstre[save].x, entite.List_monstre[save].y));}
                        else{best_deplacement.push_back(make_tuple(i, -1, -1));} 
                    }
                    else if(distance_hero > 12000 && distance_hero < 19494) // De qu il voit un monstre qui va sur base ennemie, il fonce dessus et la faudra faire des winds
                    {
                        int save = -1;
                        double distance_save = 1000000;
                        
                        // monstre le plus proche et le plus dangereux autour de chaque hero
                        for(int j = 0; j < list_id_dist_hero_M.size(); j++) // Parcours monstre autour de chaque heros
                        {
                            if(j == i)
                            {
                                for(int e = 0; e < list_id_dist_hero_M[j].size(); e++)
                                {
                                    if(get<1>(list_id_dist_hero_M[j][e]) < 2200 && entite.List_monstre[get<0>(list_id_dist_hero_M[j][e])].shield_life == 0  && (entite.List_monstre[get<0>(list_id_dist_hero_M[j][e])].near_base == 1 && entite.List_monstre[get<0>(list_id_dist_hero_M[j][e])].threat_for == 2 || entite.List_monstre[get<0>(list_id_dist_hero_M[j][e])].near_base == 0 && entite.List_monstre[get<0>(list_id_dist_hero_M[j][e])].threat_for == 2))
                                    {
                                        if(get<1>(list_id_dist_hero_M[j][e]) < distance_save)
                                        {
                                            distance_save = get<1>(list_id_dist_hero_M[j][e]);
                                            save = get<0>(list_id_dist_hero_M[j][e]);
                                        }
                                    }
                                } 
                            }
                        }

                        //Remplir tableau
                        if(save != -1){best_deplacement.push_back(make_tuple(i, entite.List_monstre[save].x, entite.List_monstre[save].y));}
                        else{best_deplacement.push_back(make_tuple(i, -1, -1));}  
                    }
                }
            }
        }

        bool determinate_if_the_time(int index_monstre, int index_hero_go_monstre)//true si j'ai le temps de le tuer, false si pas le temps
        {
            double distance = 0;
            double distance_2 = 0;
            int life = entite.List_monstre[index_monstre].health;
            const int degat = 2;

            //Recuperation distance du monstre par rapport à ma base
            for(int q = 0; q < list_id_dist_Base_allie_monstre.size(); q++)
            {
                if(index_monstre == get<0>(list_id_dist_Base_allie_monstre[q])){distance = get<1>(list_id_dist_Base_allie_monstre[q]);}
            }

            //Calcule distance hero allie pour ne pas faire n imorte quoi avec les hero opposant
            for(int q = 0; q < list_id_dist_hero_M.size(); q++)
            {
                if(index_hero_go_monstre == q)
                {
                    for(int i = 0; i < list_id_dist_hero_M[q].size(); i++)
                    {
                        if(index_monstre == get<0>(list_id_dist_hero_M[q][i])){distance_2 = get<1>(list_id_dist_hero_M[q][i]);}
                    }
                }   
            }

            //Traitements informations
            int NBtour_monstre_base = distance /400; // Calcule nb tour que mettra monstre pour aller à ma base
            int NBtour_monstre_hero = distance_2 /800; // Calcule nb tour que mettra hero pour aller sur monstre en question
            int distance_simul = (NBtour_monstre_hero * distance) / NBtour_monstre_base; // distance parcourur par monstre le temps que le hero arrive
            int distance_restant = distance - distance_simul; // Distance restante a parcourir apr monstre apres que le hero soit la 
            int tour_restant_monstre_objectif = distance_restant / 400 ; // Calcule nb tour restant apres que mon hero est joint le monstre et que le monstre est avancer en attendant
            int degat_on_monstre = (tour_restant_monstre_objectif - 1) * degat; // degat subit sur le monstre pendant le nombre de tour restant avant son objectif
            
            if(degat_on_monstre >= life){return true;}
            else{return false;}
        }

        void _best_shield()
        {
            for(int i = 0; i < 3; i++)
            {
                double distance = 0;
                int compteur = 0;
                bool activ = false;
                int Save_index_hero = -1;
                for(int q = 0; q < list_id_dist_Base_allie_hero.size(); q++)
                {
                    if(i == get<0>(list_id_dist_Base_allie_hero[q])){distance = get<1>(list_id_dist_Base_allie_hero[q]);}
                    if(q == 2){q = list_id_dist_Base_allie_hero.size();}
                }
                for(int h = 0; h < list_zone_1.size(); h++)
                {
                    if(get<0>(list_zone_1[h]) == 0){compteur++;}
                }
                
                for(int g = 0; g < list_id_dist_hero_HO.size(); g++)
                {
                    if(i == g)
                    {
                        for(int k = 0; k < list_id_dist_hero_HO[g].size(); k++)
                        {
                            if(get<1>(list_id_dist_hero_HO[g][k]) < 2200){activ = true;}
                        }
                    }
                }
                
                if(distance >= 12000)
                {
                    for(int h = 0; h < list_id_dist_hero_M.size(); h++)
                    {
                        if(h == i)
                        {
                            for(int t = 0; t < list_id_dist_hero_M[h].size(); t++)
                            {    
                                if(get<1>(list_id_dist_hero_M[h][t]) < 2200 && entite.List_monstre[get<0>(list_id_dist_hero_M[h][t])].shield_life == 0 && (entite.List_monstre[get<0>(list_id_dist_hero_M[h][t])].near_base == 0 && entite.List_monstre[get<0>(list_id_dist_hero_M[h][t])].threat_for == 2 || entite.List_monstre[get<0>(list_id_dist_hero_M[h][t])].near_base == 1 && entite.List_monstre[get<0>(list_id_dist_hero_M[h][t])].threat_for == 2)){Save_index_hero = get<0>(list_id_dist_hero_M[h][t]);}
                            }
                        }
                    }
                }
                if(Save_index_hero != -1){best_shield.push_back(make_tuple(i, 0, Save_index_hero));}//(type == 1 pour mes heros)
                else if(Save_index_hero == -1){best_shield.push_back(make_tuple(i, -1, -1));}   
            }
        }

        void sort_urgent_def_monstre(int index_monstre, int index_hero_move_dessus)
        {
            //Parcourir la liste de monstre pour chaque hero(dans la porte)
            for(int c = 0; c < list_id_dist_hero_M[index_hero_move_dessus].size(); c++)
            {
                if(index_monstre == get<0>(list_id_dist_hero_M[index_hero_move_dessus][c]) && get<1>(list_id_dist_hero_M[index_hero_move_dessus][c]) <= 1280 && entite.List_monstre[get<0>(list_id_dist_hero_M[index_hero_move_dessus][c])].shield_life == 0)
                {
                    choose_sort.push_back(make_tuple("WIND", base.List_Base[1].base_x, base.List_Base[1].base_y, -1, index_hero_move_dessus));
                }
                else if(index_monstre == get<0>(list_id_dist_hero_M[index_hero_move_dessus][c]) && get<1>(list_id_dist_hero_M[index_hero_move_dessus][c]) <= 2200 && entite.List_monstre[get<0>(list_id_dist_hero_M[index_hero_move_dessus][c])].shield_life == 0)
                {
                    choose_sort.push_back(make_tuple("CONTROL", base.List_Base[1].base_x, base.List_Base[1].base_y, index_monstre, index_hero_move_dessus));
                }
            }
        }

        void choose_best_wind()//ok
        {
            const int monstre = 0;
            const int hero_opposent = 2;

            for(int i = 0; i < 3; i++) // Parcourir les 3 heros
            {
                vector<tuple<int,int>> monstre_porte; // Type(0 monstre, 2 hero opposent) et index
                vector<tuple<bool,int,int,int>> Save; //autorise ou pas(true,false), index(quel hero), x et y direction
                double distance = 0;
                bool presence_danger = false;
                int compteur_monstre = 0;
                int compteur_hero_oppo = 0;
                bool situationoffensive = false;

                //Parcourir la liste de monstre pour chaque hero(dans la porte)
                for(int c = 0; c < list_id_dist_hero_M[i].size(); c++)
                {
                    if(get<1>(list_id_dist_hero_M[i][c]) <= 1280 && entite.List_monstre[get<0>(list_id_dist_hero_M[i][c])].shield_life == 0)
                    {
                        monstre_porte.push_back(make_tuple(monstre, get<0>(list_id_dist_hero_M[i][c])));
                    }
                }

                //Parcourir la liste de hero oppposent pour chaque hero(dans la porte)
                for(int c = 0; c < list_id_dist_hero_HO[i].size(); c++)
                {
                    if(get<1>(list_id_dist_hero_HO[i][c]) <= 1280 && entite.List_hero_Ennemie[get<1>(list_id_dist_hero_HO[i][c])].shield_life == 0)
                    {
                        monstre_porte.push_back(make_tuple(hero_opposent, get<0>(list_id_dist_hero_HO[i][c])));
                    }
                }

                //Calcule distance hero allie pour ne pas faire n imorte quoi avec les hero opposant
                for(int q = 0; q < list_id_dist_Base_allie_hero.size(); q++)
                {
                    if(i == get<0>(list_id_dist_Base_allie_hero[q])){distance = get<1>(list_id_dist_Base_allie_hero[q]);}
                    if(q == 2){q = list_id_dist_Base_allie_hero.size();}
                }
                
                // Traitements informations
                if(distance > 300 && distance < 2800) //Phase defense 1
                {
                    if(monstre_porte.size() > 0)
                    {
                       //contre mur
                       presence_danger = true;
                    }
                }

                if(distance > 2900 && distance < 5000) //Phase defense 2
                {
                    if(monstre_porte.size() > 0)
                    {
                       //vers hero base ennemie
                       presence_danger = true;
                    }
                }

                if(distance > 300 && distance < 5000) // Contre hero ennemie
                {
                    for(int y = 0; y < monstre_porte.size(); y++)
                    {
                        if(get<0>(monstre_porte[y]) == 0){compteur_monstre++;}
                        else{compteur_hero_oppo++;}
                    }

                    int compteur_2 = 0;
                    for(int y = 0; y < list_id_dist_Base_allie_monstre.size(); y++)
                    {
                        if(get<1>(list_id_dist_Base_ennemie_monstre[y]) < 5000)
                        {
                            compteur_2++;
                        }
                    }
                    if(compteur_hero_oppo > 0 && compteur_2 > 0){presence_danger = true;}   
                }

                if(distance > 5000 && distance < 9000) //Phase defense 3
                {
                    if(monstre_porte.size() > 3)
                    {
                       //vers hero base ennemie
                       presence_danger = true;
                    }
                }

                if(distance > 11800 && distance < 14000) //Phase offensive 1
                {
                    for(int y = 0; y < monstre_porte.size(); y++)
                    {
                        if(get<0>(monstre_porte[y]) == 0){compteur_monstre++;}
                        else{compteur_hero_oppo++;}
                    }
                    if(compteur_monstre >= compteur_hero_oppo && compteur_monstre !=0)
                    {
                       //Vers base ennemie
                       presence_danger = true;
                       compteur_monstre = 0;
                       compteur_hero_oppo = 0;
                    }
                }

                if(distance > 14000 && distance < 19494) //Phase offensive 2
                {
                    for(int y = 0; y < monstre_porte.size(); y++)
                    {
                        if(get<0>(monstre_porte[y]) == 0){compteur_monstre++;}
                        else{compteur_hero_oppo++;}
                    }
                    if(compteur_monstre >= compteur_hero_oppo && compteur_monstre !=0)
                    {
                       //Vers base ennemie
                       presence_danger = true;
                       compteur_monstre = 0;
                       compteur_hero_oppo = 0;
                    }
                    else if(compteur_hero_oppo > 0 && compteur_monstre == 0)
                    {
                        int compteur_2 = 0;
                        for(int y = 0; y < list_id_dist_Base_ennemie_monstre.size(); y++)
                        {
                            if(get<1>(list_id_dist_Base_ennemie_monstre[y]) < 5000)
                            {
                                compteur_2++;
                            }
                        }
                        if(compteur_2 > 0){presence_danger = true;situationoffensive = true;}
                    }
                }

                if(situationoffensive == false){Save.push_back(make_tuple(presence_danger,i,base.List_Base[1].base_x,base.List_Base[1].base_y));}
                else{Save.push_back(make_tuple(presence_danger,i,base.List_Base[0].base_x,base.List_Base[0].base_y));}

                for(int h = 0; h < monstre_porte.size(); h++)
                {
                    if(get<0>(monstre_porte[h]) == 0){Save.push_back(make_tuple(0,get<1>(monstre_porte[h]),-1,-1));} //0 monstre
                    else{Save.push_back(make_tuple(1,get<1>(monstre_porte[h]),-1,-1));} //hero opposent
                }
                
                autorisation_wind.push_back(Save);
            }
        }

        void choose_best_Constrol_hero()
        {
            const int monstre = 0;
            const int hero_opposent = 2;

            for(int i = 0; i < 3; i++)//parcours de mes trois heros
            {
                vector<tuple<int,int>> monstre_porte; // Type(0 monstre, 2 hero opposent) et index
                vector<tuple<int,int,int,int>> stockage; // x y type index
                double distance = 0;

                //Parcourir la liste de monstre pour chaque hero(dans la porte)
                for(int c = 0; c < list_id_dist_hero_M[i].size(); c++)
                {
                    if(get<1>(list_id_dist_hero_M[i][c]) <= 2200 && entite.List_monstre[get<0>(list_id_dist_hero_M[i][c])].shield_life == 0)
                    {
                        monstre_porte.push_back(make_tuple(monstre, get<0>(list_id_dist_hero_M[i][c])));
                    }
                }

                //Parcourir la liste de hero oppposent pour chaque hero(dans la porte)
                for(int c = 0; c < list_id_dist_hero_HO[i].size(); c++)
                {
                    if(get<1>(list_id_dist_hero_HO[i][c]) <= 2200 && entite.List_hero_Ennemie[get<0>(list_id_dist_hero_HO[i][c])].shield_life == 0)
                    {
                        monstre_porte.push_back(make_tuple(hero_opposent, get<0>(list_id_dist_hero_HO[i][c])));
                    }
                }

                //Calcule distance hero allie pour ne pas faire n imorte quoi avec les hero opposant
                for(int q = 0; q < list_id_dist_Base_allie_hero.size(); q++)
                {
                    if(i == get<0>(list_id_dist_Base_allie_hero[q])){distance = get<1>(list_id_dist_Base_allie_hero[q]);}
                    if(q == 2){q = list_id_dist_Base_allie_hero.size();}
                }

                //Traitement information stocke(defensif, offensif)
                for(int f = 0; f < 2; f++)
                {
                    if(i == 2){std::clock_t c_end = std::clock();double time_elapsed_ms = 1000.0 * (c_end-debut) / CLOCKS_PER_SEC;cerr << "CPU time used: " << time_elapsed_ms << " ms" << endl;}

                    double max = 0;
                    double max_2 = 0;
                    int value_Save = -1; // -1 si il n' y a rien
                    int value_Save_2 = -1;
                    bool activ = false;
                    bool activ_2 = false;
                    bool activ_3 = false;
                   
                    if(f == 0) // defense
                    {
                        for(int c = 0; c < monstre_porte.size(); c++) // Phase 1 pour chercher le score le plus eleve pour la defense
                        {
                            bool activ_4 = false;
                            int compteur_2 = 0;
                            int distance_2 = 0;

                            //Calcule distance monstre base 4600 a voir pour changer
                            if(get<0>(monstre_porte[c]) == 0)
                            {for(int y = 0; y < list_id_dist_Base_allie_monstre.size(); y++){
                            if(get<0>(list_id_dist_Base_allie_monstre[y]) == get<1>(monstre_porte[c])){distance_2 = get<1>(list_id_dist_Base_allie_monstre[y]);}}}

                            //compter le nombre de monstre entre ma base et 6000 et savoir si hero allie s'y trouve pour defendre et que hero ennemie a a porter le hero allie en question
                            if(get<0>(monstre_porte[c]) == 2 && distance < 9400)
                            {
                                for(int q = 0; q < list_id_dist_Base_allie_hero.size(); q++)
                                {
                                    if(get<1>(list_id_dist_Base_allie_hero[q]) < 5000)//6000
                                    {
                                        for(int k = 0; k < list_id_dist_hero_HO.size(); k++)
                                        {
                                            if(k == get<0>(list_id_dist_Base_allie_hero[q]))
                                            {
                                                for(int t = 0; t < list_id_dist_hero_HO[k].size(); t++)
                                                {
                                                    if(get<1>(list_id_dist_hero_HO[k][t]) < 2200)
                                                    {
                                                        activ_4 = true;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    if(q == 2){q = list_id_dist_Base_allie_hero.size();}
                                }
                                for(int q = 0; q < list_id_dist_Base_allie_monstre.size(); q++)
                                {
                                    if(get<1>(list_id_dist_Base_allie_monstre[q]) < 5000)//6000
                                    {
                                        compteur_2++;
                                    }
                                }
                            }

                            if(get<0>(monstre_porte[c]) != 2 && distance_2 > 4600 && distance_2 < 9400 && entite.List_monstre[get<1>(monstre_porte[c])].level_danger > max && (entite.List_monstre[get<1>(monstre_porte[c])].near_base == 0 && entite.List_monstre[get<1>(monstre_porte[c])].threat_for == 1 ||  entite.List_monstre[get<1>(monstre_porte[c])].near_base == 1 && entite.List_monstre[get<1>(monstre_porte[c])].threat_for == 1))
                            {
                                max = entite.List_monstre[get<1>(monstre_porte[c])].level_danger;
                                value_Save = get<1>(monstre_porte[c]);
                            }
                            else if(get<0>(monstre_porte[c]) == 2 && (entite.List_hero_Ennemie[get<1>(monstre_porte[c])].level_danger > max_2 && entite.List_hero_Ennemie[get<1>(monstre_porte[c])].level_danger > 1 && activ_3 == false && activ_4 == false && compteur_2 < 2 || activ_3 == false && activ_4 == true && compteur_2 > 1 && base.List_Base[1].mana >= 10))
                            {
                                if(compteur_2 > 1 && activ_4 == true){value_Save_2 = get<1>(monstre_porte[c]);  activ_3 = true;}
                                else{max_2 = entite.List_hero_Ennemie[get<1>(monstre_porte[c])].level_danger;value_Save_2 = get<1>(monstre_porte[c]);} 
                            }
                        }  
                    }
                    else // offensive coord_best_Controle
                    {
                        for(int c = 0; c < monstre_porte.size(); c++) // Phase 2 offensive
                        {
                            bool activ_3 = false;
                            int compteur = 0;

                            //Calcule nombre monstre ennemie qui sont entre 0 et 6000(par rapport a base ennemie) et qui vont sur base ennemie et si hero opposent est aussi dans cette zone
                            if(get<0>(monstre_porte[c]) == 2 && distance >= 12000)
                            {
                                for(int j = 3; j < list_id_dist_Base_ennemie_hero.size(); j++)
                                {
                                    if(entite.List_hero_Ennemie[get<0>(list_id_dist_Base_ennemie_hero[j])].id == entite.List_hero_Ennemie[get<1>(monstre_porte[c])].id && get<1>(list_id_dist_Base_ennemie_hero[j]) < 5000)//6000
                                    {
                                        activ_3 = true;
                                    }

                                    if(activ_3 == true){j = list_id_dist_Base_ennemie_hero.size();}
                                }
                                for(int t = 0; t < list_id_dist_Base_ennemie_monstre.size(); t++)
                                {
                                    if(get<1>(list_id_dist_Base_ennemie_monstre[t]) < 5000)//6000
                                    {
                                        compteur++;
                                    }
                                }
                            }
                            if(activ != true && get<0>(monstre_porte[c]) == 0 && distance > 9400 &&  (entite.List_monstre[get<1>(monstre_porte[c])].near_base == 0 && (entite.List_monstre[get<1>(monstre_porte[c])].threat_for == 0 || entite.List_monstre[get<1>(monstre_porte[c])].threat_for == 1) ||  entite.List_monstre[get<1>(monstre_porte[c])].near_base == 1 && entite.List_monstre[get<1>(monstre_porte[c])].threat_for == 1))
                            {
                                value_Save = get<1>(monstre_porte[c]);
                                activ = true;
                            }
                            else if(activ_2 != true && get<0>(monstre_porte[c]) == 2 && activ_3 == true && compteur > 1)//Voir pour ajuste le nombre de monstre qu on veut dans cette zone pour le sort
                            {
                                value_Save_2 = get<1>(monstre_porte[c]);
                                activ_2 = true;
                            }
                            if(activ == true && activ_2 == true){c = monstre_porte.size();}
                        }
                    }

                    // Ajout  defense(monstre, hero) et offensive(monstre, hero)
                    if(f == 0){stockage.push_back(make_tuple(base.List_Base[1].base_x,base.List_Base[1].base_y, monstre, value_Save));
                    stockage.push_back(make_tuple(base.List_Base[1].base_x,base.List_Base[1].base_y, hero_opposent, value_Save_2));}
                    else{stockage.push_back(make_tuple(base.List_Base[1].base_x,base.List_Base[1].base_y, monstre, value_Save));
                    stockage.push_back(make_tuple(base.List_Base[0].base_x,base.List_Base[0].base_y, hero_opposent, value_Save_2));}
                }
                //Stoackage du vecteur de tuple dans le tableau finale
                coord_best_Controle.push_back(stockage);
            }
        }
        
        // Verifier etat  zone 1 (max 5000)
        // Renvoi le nombre d'entite presente(type, index tableau) , seulement hero_opposent et monstre
        void verif_Etat_zone() 
        {
            int monstre = 0;
            int hero_opposent = 2;
            int zone = 0;
            int taille = list_id_dist_Base_allie_monstre.size();
            int taille_2 = list_id_dist_Base_allie_hero.size();
            
            for(int j = 0; j<3; j++) // Faire les 3 zones
            {
                if(j == 0){zone = ZONE_1_max_A;}
                else if(j == 1){zone = ZONE_2_max_A;}
                else{zone = ZONE_3_max_E; taille = list_id_dist_Base_ennemie_monstre.size();taille_2 = list_id_dist_Base_ennemie_hero.size();}

                //Chercher monstre present
                for(auto i = 0; i<taille; i++)
                {
                    if(j == 0)
                    {
                        if(get<1>(list_id_dist_Base_allie_monstre[i]) < zone)
                        {
                            list_zone_1.push_back(make_tuple(monstre,get<0>(list_id_dist_Base_allie_monstre[i])));
                        }
                    }
                    else if(j == 1)
                    {
                        if(get<1>(list_id_dist_Base_allie_monstre[i]) < zone && get<1>(list_id_dist_Base_allie_monstre[i]) > 5000)
                        {
                            list_zone_2.push_back(make_tuple(monstre,get<0>(list_id_dist_Base_allie_monstre[i])));
                        }
                    }
                    else
                    {
                        if(get<1>(list_id_dist_Base_ennemie_monstre[i]) < zone)
                        {
                            list_zone_3.push_back(make_tuple(monstre,get<0>(list_id_dist_Base_ennemie_monstre[i])));
                        }
                    }
                }

                //Chercher hero_opposent present
                for(auto i = 3; i<taille_2; i++)
                {
                    if(j == 0)
                    {
                        if(get<1>(list_id_dist_Base_allie_hero[i]) < zone)
                        {
                            list_zone_1.push_back(make_tuple(hero_opposent,get<0>(list_id_dist_Base_allie_hero[i])));
                        }
                    }
                    else if(j == 1)
                    {
                        if(get<1>(list_id_dist_Base_allie_hero[i]) < zone && get<1>(list_id_dist_Base_allie_hero[i]) > 5000)
                        {
                            list_zone_2.push_back(make_tuple(hero_opposent,get<0>(list_id_dist_Base_allie_hero[i])));
                        }
                    }
                    else
                    {
                        if(get<1>(list_id_dist_Base_ennemie_hero[i]) < zone)
                        {
                            list_zone_3.push_back(make_tuple(hero_opposent,get<0>(list_id_dist_Base_ennemie_hero[i])));
                        }
                    }
                }
            }  
        } 

        void evaluation_situation_monstre_hero_opposent_visible(int index = 0) //Le score le plus faible est le moins dangereux pour ma base(le plusdangereux pour moi, le plus dangereux pour l ennemie se fera directe dans les fonction de sort)
        {
            const int level = 0;
            const int level_0 = 1; // moins dangereux au plus dangereux
            const int level_1 = 2;
            const int level_2 = 3;
            const int level_3 = 4;
            const int level_4 = 5;

            //Taux penalite attribuer pour le shiel life
            const float shiel_life_never = 0.0; 
            const float shiel_life_level = 0.2; // moins dangereux au plus dangereux
            const float shiel_life_level_0 = 0.4; 
            const float shiel_life_level_1 = 0.6;
            const float shiel_life_level_2 = 0.8;
            const float shiel_life_level_3 = 1.0; // Pas utilise

            // Taux penalite pour is control
            const float is_Control_never = 0.0;
            const float is_Control_ = 0.1;

            // taux penalite point de vie
            const float vie = 0;
            const float vie_1 = 0.1;

            // Penalite pour near_base et threat
            const float N_0 = 0.0;
            const float N_1 = 0.4;
            const float N_2 = 0.8;
            const float N_3 = 1.2;

            //Evaluation monstre---------------------
            for(auto i = 0; i<entite.List_monstre.size(); i++)
            {
                //Attribution penalite pour la valeur de shield life
                if(entite.List_monstre[i].shield_life == 0){entite.List_monstre[i].level_danger += shiel_life_never;}
                else if(entite.List_monstre[i].shield_life < 3){entite.List_monstre[i].level_danger += shiel_life_level;}
                else if(entite.List_monstre[i].shield_life > 2 && entite.List_monstre[i].shield_life < 6){entite.List_monstre[i].level_danger += shiel_life_level_0;}
                else if(entite.List_monstre[i].shield_life > 5 && entite.List_monstre[i].shield_life < 9){entite.List_monstre[i].level_danger += shiel_life_level_1;}
                else if(entite.List_monstre[i].shield_life > 8 && entite.List_monstre[i].shield_life < 12){entite.List_monstre[i].level_danger += shiel_life_level_2;}
                
                //Attribution penalite pour la valeur is constrolled
                if(entite.List_monstre[i].is_controlled == 0){entite.List_monstre[i].level_danger += is_Control_never;}
                else{entite.List_monstre[i].level_danger += is_Control_;}

                //Attribution penalite pour la valeur health
                if(entite.List_monstre[i].health > 2){entite.List_monstre[i].level_danger += vie_1;}
                else{entite.List_monstre[i].level_danger += vie;}

                //Attribution penalite pour la valeur vx, vy
                //...

                //Attribution penalite pour la valeur near_base et threat_for
                if(entite.List_monstre[i].near_base == 1 && entite.List_monstre[i].threat_for == 1){entite.List_monstre[i].level_danger += N_3;}
                else if(entite.List_monstre[i].near_base == 1 && entite.List_monstre[i].threat_for == 2){entite.List_monstre[i].level_danger += N_0;}
                else if(entite.List_monstre[i].near_base == 0 && entite.List_monstre[i].threat_for == 0){entite.List_monstre[i].level_danger += N_0;}
                else if(entite.List_monstre[i].near_base == 0 && entite.List_monstre[i].threat_for == 1){entite.List_monstre[i].level_danger += N_2;}
                else if(entite.List_monstre[i].near_base == 0 && entite.List_monstre[i].threat_for == 2){entite.List_monstre[i].level_danger += N_0;}

                //Distance base
                int save1 = 0;
                int save2 = 0;
                int dist_stock = 0;
                for(auto j = 0; j<list_id_dist_Base_allie_monstre.size(); j++)
                {
                    if(i == get<0>(list_id_dist_Base_allie_monstre[j]))
                    {
                        dist_stock = get<1>(list_id_dist_Base_allie_monstre[j]);
                        for(int f = 1; f<20; f++)
                        {
                            save1 = f*1000;
                            if(get<1>(list_id_dist_Base_allie_monstre[j]) > save2 && get<1>(list_id_dist_Base_allie_monstre[j]) < save1)
                            {save2 = save1; int valeur = (19 - f)/10; entite.List_monstre[i].level_danger += valeur;}
                        }
                        break; 
                    }
                }

                //Calcule prochain mouvement avec vx et vy pour la penalite
                //Calcule prochaine distance
                double dist_1 = sqrt(pow(base.List_Base[0].base_x - (entite.List_monstre[i].x + entite.List_monstre[i].vx) , 2) + pow(base.List_Base[0].base_y - (entite.List_monstre[i].y + entite.List_monstre[i].vy), 2));
                if(dist_1 < 5000){entite.List_monstre[i].level_danger += 1.0;}
                
                //Penalite pour le nombre de tour restant à faire pour arriver sur ma base(si il me vise)
                int dist_test = 0;
                if(entite.List_monstre[i].near_base == 1 && entite.List_monstre[i].threat_for == 1 || entite.List_monstre[i].near_base == 0 && entite.List_monstre[i].threat_for == 1)
                {
                    dist_test = dist_stock/400;
                    if(dist_test < 3){entite.List_monstre[i].level_danger += 1.0;}
                    else if(dist_test > 2 && dist_test < 6){entite.List_monstre[i].level_danger += 1.0;}
                    else if(dist_test > 5 && dist_test < 9){entite.List_monstre[i].level_danger += 0.8;}
                    else if(dist_test > 8 && dist_test < 12){entite.List_monstre[i].level_danger += 0.6;}
                    else if(dist_test > 11 && dist_test < 15){entite.List_monstre[i].level_danger += 0.4;}
                    else{entite.List_monstre[i].level_danger += 0.2;}
                    
                }
                
                //cerr << "mana ennemie : " << base.List_Base[1].mana << endl;
                //cerr << "mana_old ennemie : " << base.List_Base[1].mana_old << endl;
                //Penalite si hero opposent assez proche d un monstre pour faire un wind et que le monstre va dans ma base et qu il est proche
                bool porte_hero_opp = false;
                for(int r = 0; r<list_id_distance_hero_opposent_monstre.size(); r++)
                {
                    for(int g = 0; g<list_id_distance_hero_opposent_monstre[r].size(); g++)
                    {
                        if(get<0>(list_id_distance_hero_opposent_monstre[r][g]) == i && get<1>(list_id_distance_hero_opposent_monstre[r][g]) < 1280 && dist_stock > 7200 && dist_stock <= 11200 && (entite.List_monstre[i].near_base == 1 && entite.List_monstre[i].threat_for == 1 || entite.List_monstre[i].near_base == 0 && entite.List_monstre[i].threat_for == 1) && base.List_Base[1].mana >= 10)
                        {
                            entite.List_monstre[i].level_danger += 0.1;
                            //cerr << "activ" << endl;
                            porte_hero_opp = true;
                        }
                        else if(get<0>(list_id_distance_hero_opposent_monstre[r][g]) == i && get<1>(list_id_distance_hero_opposent_monstre[r][g]) < 1280 && dist_stock <= 7200 && base.List_Base[1].mana >= 10)
                        {
                            entite.List_monstre[i].level_danger += 0.1;//a voir pour ajuster
                            //cerr << "activ" << endl;
                            porte_hero_opp = true;
                        }
                        //Boucle sortie
                        if(porte_hero_opp == true){g = list_id_distance_hero_opposent_monstre[r].size();}
                    }
                    if(porte_hero_opp == true){r = list_id_distance_hero_opposent_monstre.size();}
                }

                //Penalite si hero opposent assez proche d un monstre pour faire un control et que le monstre va dans ma base et qu il est proche
                bool porte_hero_opp_2 = false;//Voir pour enlever condidtion changement mana...
                for(int r = 0; r<list_id_distance_hero_opposent_monstre.size(); r++)
                {
                    for(int g = 0; g<list_id_distance_hero_opposent_monstre[r].size(); g++)
                    {
                        if(get<0>(list_id_distance_hero_opposent_monstre[r][g]) == i && get<1>(list_id_distance_hero_opposent_monstre[r][g]) < 2200 && dist_stock > 7200 && dist_stock <= 11200 && (entite.List_monstre[i].near_base == 1 && entite.List_monstre[i].threat_for == 2 || entite.List_monstre[i].near_base == 0 && (entite.List_monstre[i].threat_for == 0 || entite.List_monstre[i].threat_for == 2)) && base.List_Base[1].mana >=10)
                        {
                            entite.List_monstre[i].level_danger += 0.1;
                            //cerr << "activ" << endl;
                            porte_hero_opp_2 = true;
                        }
                        else if(get<0>(list_id_distance_hero_opposent_monstre[r][g]) == i && get<1>(list_id_distance_hero_opposent_monstre[r][g]) < 2200 && dist_stock <= 7200 && (entite.List_monstre[i].near_base == 1 && entite.List_monstre[i].threat_for == 2 || entite.List_monstre[i].near_base == 0 && (entite.List_monstre[i].threat_for == 0 || entite.List_monstre[i].threat_for == 2)) && base.List_Base[1].mana >= 10)
                        {
                            entite.List_monstre[i].level_danger += 0.1;// a voir pour ajuster
                            //cerr << "activ" << endl;
                            porte_hero_opp_2 = true;
                        }
                        //Boucle sortie
                        if(porte_hero_opp_2 == true){g = list_id_distance_hero_opposent_monstre[r].size();}
                    }
                    if(porte_hero_opp_2 == true){r = list_id_distance_hero_opposent_monstre.size();}
                }
            }

            //Hero opposent-------------------------------------------------------------------------
            for(int f = 3; f<list_id_dist_Base_allie_hero.size(); f++)
            {
                int dist_stock = get<1>(list_id_dist_Base_allie_hero[f]);
                //Penalite distance hero oppenset et ma base
                if(get<1>(list_id_dist_Base_allie_hero[f]) < ZONE_2_max_A)
                {
                    entite.List_hero_Ennemie[get<0>(list_id_dist_Base_allie_hero[f])].level_danger += 1.0;
                    //cerr << "danger zone hero ennemie :" << entite.List_hero_Ennemie[get<0>(list_id_dist_Base_allie_hero[f])].id << " /danger" <<  entite.List_hero_Ennemie[get<0>(list_id_dist_Base_allie_hero[f])].level_danger << endl;
                }

                //Pénalite en fonction du sort wind
                //Attribution penalite pour la valeur de shield life
                if(entite.List_hero_Ennemie[get<0>(list_id_dist_Base_allie_hero[f])].shield_life == 0){entite.List_hero_Ennemie[get<0>(list_id_dist_Base_allie_hero[f])].level_danger += shiel_life_never;}
                else if(entite.List_hero_Ennemie[get<0>(list_id_dist_Base_allie_hero[f])].shield_life < 3){entite.List_hero_Ennemie[get<0>(list_id_dist_Base_allie_hero[f])].level_danger += shiel_life_level;}
                else if(entite.List_hero_Ennemie[get<0>(list_id_dist_Base_allie_hero[f])].shield_life > 2 && entite.List_hero_Ennemie[get<0>(list_id_dist_Base_allie_hero[f])].shield_life < 6){entite.List_hero_Ennemie[get<0>(list_id_dist_Base_allie_hero[f])].level_danger += shiel_life_level_0;}
                else if(entite.List_hero_Ennemie[get<0>(list_id_dist_Base_allie_hero[f])].shield_life > 5 && entite.List_hero_Ennemie[get<0>(list_id_dist_Base_allie_hero[f])].shield_life < 9){entite.List_hero_Ennemie[get<0>(list_id_dist_Base_allie_hero[f])].level_danger += shiel_life_level_1;}
                else if(entite.List_hero_Ennemie[get<0>(list_id_dist_Base_allie_hero[f])].shield_life > 8 && entite.List_hero_Ennemie[get<0>(list_id_dist_Base_allie_hero[f])].shield_life < 12){entite.List_hero_Ennemie[get<0>(list_id_dist_Base_allie_hero[f])].level_danger += shiel_life_level_2;}
                
                //Penalite si hero opposent assez proche d un monstre pour faire un wind et que le monstre va dans ma base et qu il est proche
                bool porte_hero_opp = false;
                for(int r = 0; r<list_id_distance_hero_opposent_monstre.size(); r++)
                {
                    if(f - 3 == r)
                    {
                        for(int g = 0; g<list_id_distance_hero_opposent_monstre[r].size(); g++)
                        {
                            if(get<1>(list_id_distance_hero_opposent_monstre[r][g]) < 1280 && dist_stock > 7200 && dist_stock <= 11200 && (entite.List_monstre[get<0>(list_id_distance_hero_opposent_monstre[r][g])].near_base == 1 && entite.List_monstre[f].threat_for == 1 || entite.List_monstre[get<0>(list_id_distance_hero_opposent_monstre[r][g])].near_base == 0 && entite.List_monstre[get<0>(list_id_distance_hero_opposent_monstre[r][g])].threat_for == 1) && base.List_Base[1].mana >= 10)
                            {
                                entite.List_hero_Ennemie[get<0>(list_id_dist_Base_allie_hero[f])].level_danger += 0.1;
                                //cerr << "activ1" << endl;
                            }
                            else if(get<1>(list_id_distance_hero_opposent_monstre[r][g]) < 1280 && dist_stock <= 7200 && base.List_Base[1].mana >= 10)
                            {
                                entite.List_hero_Ennemie[get<0>(list_id_dist_Base_allie_hero[f])].level_danger += 0.2;
                                //cerr << "activ1" << endl;
                            }
                            //Boucle sortie
                            if(g == list_id_distance_hero_opposent_monstre[r].size() - 1){;porte_hero_opp = true;}
                        }
                        if(porte_hero_opp == true){r = list_id_distance_hero_opposent_monstre.size();}
                    } 
                }
                //Penalite si hero opposent assez proche d un monstre pour faire un control et que le monstre va dans ma base et qu il est proche
                bool porte_hero_opp_2 = false;
                for(int r = 0; r<list_id_distance_hero_opposent_monstre.size(); r++)
                {
                    if(f - 3 == r)
                    {
                        for(int g = 0; g<list_id_distance_hero_opposent_monstre[r].size(); g++)
                        {
                            if(get<1>(list_id_distance_hero_opposent_monstre[r][g]) < 2200 && dist_stock > 7200 && dist_stock <= 11200 && (entite.List_monstre[get<0>(list_id_distance_hero_opposent_monstre[r][g])].near_base == 1 && entite.List_monstre[get<0>(list_id_distance_hero_opposent_monstre[r][g])].threat_for == 2 || entite.List_monstre[get<0>(list_id_distance_hero_opposent_monstre[r][g])].near_base == 0 && (entite.List_monstre[get<0>(list_id_distance_hero_opposent_monstre[r][g])].threat_for == 0 || entite.List_monstre[get<0>(list_id_distance_hero_opposent_monstre[r][g])].threat_for == 2)) && base.List_Base[1].mana >= 10)
                            {
                                entite.List_hero_Ennemie[get<0>(list_id_dist_Base_allie_hero[f])].level_danger += 0.1;
                                //cerr << "activ2" << endl;
                            }
                            else if(get<1>(list_id_distance_hero_opposent_monstre[r][g]) < 2200 && dist_stock <= 7200 && (entite.List_monstre[get<0>(list_id_distance_hero_opposent_monstre[r][g])].near_base == 1 && entite.List_monstre[get<0>(list_id_distance_hero_opposent_monstre[r][g])].threat_for == 2 || entite.List_monstre[get<0>(list_id_distance_hero_opposent_monstre[r][g])].near_base == 0 && (entite.List_monstre[get<0>(list_id_distance_hero_opposent_monstre[r][g])].threat_for == 0 || entite.List_monstre[get<0>(list_id_distance_hero_opposent_monstre[r][g])].threat_for == 2)) && base.List_Base[1].mana >= 10)
                            {
                                entite.List_hero_Ennemie[get<0>(list_id_dist_Base_allie_hero[f])].level_danger += 0.2;
                                //cerr << "activ2" << endl;
                            }
                            //Boucle sortie
                            if(g == list_id_distance_hero_opposent_monstre[r].size() - 1){;porte_hero_opp = true;}
                        }
                        if(porte_hero_opp == true){r = list_id_distance_hero_opposent_monstre.size();}
                    } 
                }
            }
            //.... 
        }

        void fint_next_monstre_direction_base()
        {
           

            
        } 
        
        /* 
         * Calcule toutes les distance entre la base allie et monstre(idem pour base ennemie), puis elle trie dans l'ordre croissant
         * je recupere ces valeurs dans des tuples contenant distance et id(id du tableau et pas id dans la structure car plus simple using)
         * @param int nb_value (passer la taille max des monstres pour avoir toutes les distances), la on peut se permettre de choisir les quellle on veut
         * returtn rien
        */
        void calcule_distance_monstre_base(int nb_value)
        {
            for(int e = 0; e<2; e++)
            {
                int save = 0;
                double save_2 = 0;
                int save_3 = 0;
                int compteur = 0;

                vector<double> Save_Min_dist;
                
                if(e == 0){list_id_dist_Base_allie_monstre.resize(nb_value);}
                else{list_id_dist_Base_ennemie_monstre.resize(nb_value);}
                
                Save_Min_dist.resize(nb_value);

                for(auto &i : Save_Min_dist){i = i + 10000000;}

                for(int i = 0; i<entite.List_monstre.size(); i++)
                {
                    double dist_1 = sqrt(pow(base.List_Base[e].base_x - entite.List_monstre[i].x, 2) + pow(base.List_Base[e].base_y - entite.List_monstre[i].y, 2));
                    int buffer = i; //entite.List_monstre[i].id
                    int buffer_2 = dist_1;

                    for(int j = 0; j<Save_Min_dist.size(); j++)
                    {
                        if(dist_1 < Save_Min_dist[j])
                        {
                            if(e == 0){save = get<0>(list_id_dist_Base_allie_monstre[j]); get<0>(list_id_dist_Base_allie_monstre[j]) = buffer;} // Save ancien id // Save new id  
                            else{save = get<0>(list_id_dist_Base_ennemie_monstre[j]); get<0>(list_id_dist_Base_ennemie_monstre[j]) = buffer;}
                            buffer = save; // nouvelle id pour prochain tour 
 
                            if(e == 0){save_3 = get<1>(list_id_dist_Base_allie_monstre[j]); get<1>(list_id_dist_Base_allie_monstre[j]) = buffer_2;} // Save ancien dist // Save new dist  
                            else{save_3 = get<1>(list_id_dist_Base_ennemie_monstre[j]); get<1>(list_id_dist_Base_ennemie_monstre[j]) = buffer_2;}
                            buffer_2 = save_3; // nouvelle dist pour prochain tour 

                            save_2 = Save_Min_dist[j]; // Save min actuel 
                            Save_Min_dist[j] = dist_1; // Save new min 
                            dist_1 = save_2; // Save new dist avec celle qu on descend  
                        }
                    }
                }
            }  
        }
        
        /* 
         * Calcule toutes les distance entre la base allie et hero (idem pour base ennemie), puis elle trie dans l'ordre croissant
         * je recupere ces valeurs dans des tuples contenant distance et id(id du tableau et pas id dans la structure car plus simple using)
         * les deux tableaux pour base allie et opposent : 3 premier valeur(hero), le reste(hero opposent)
         * @param int nb_value(il faut lui passer la taille max du tableau contenant les heros allie // alway == 3->fait pour 3 car bug sur compteur sinon) car sinon sa ne marchera pas(pb avec compteur = 3 sinon)
         * returtn rien
        */
        void calcule_distance_hero_base(int nb_value)//
        {
            for(int k = 0; k<2; k++)
            {
                // hero Allie
                int save = 0;
                double save_2 = 0;
                int save_3 = 0;
                int compteur = 0;

                vector<double> Save_Min_dist;
                
                if(k == 0)
                {
                    list_id_dist_Base_allie_hero.resize(nb_value + entite.List_hero_Ennemie.size());
                }
                else
                {
                    list_id_dist_Base_ennemie_hero.resize(nb_value + entite.List_hero_Ennemie.size());
                }
                
                Save_Min_dist.resize(nb_value);

                for(auto &i : Save_Min_dist){i = i + 10000000;}

                for(int i = 0; i<entite.List_hero_Allie.size(); i++)
                {
                    double dist_1 = sqrt(pow(base.List_Base[k].base_x - entite.List_hero_Allie[i].x, 2) + pow(base.List_Base[k].base_y - entite.List_hero_Allie[i].y, 2));
                    int buffer = i; //entite.List_monstre[i].id
                    int buffer_2 = dist_1;

                    for(int j = 0; j<Save_Min_dist.size(); j++)
                    {
                        if(dist_1 < Save_Min_dist[j])
                        {
                            if(k == 0)
                            {
                                save = get<0>(list_id_dist_Base_allie_hero[j]);
                                get<0>(list_id_dist_Base_allie_hero[j]) = buffer; // Save ancien id // Save new id  
                                buffer = save; // nouvelle id pour prochain tour 

                                save_3 = get<1>(list_id_dist_Base_allie_hero[j]); // Save dist ancien
                                get<1>(list_id_dist_Base_allie_hero[j]) = buffer_2; // Save new id  
                                buffer_2 = save_3; // nouvelle dist pour prochain tour 
                            }
                            else
                            {
                                save = get<0>(list_id_dist_Base_ennemie_hero[j]);
                                get<0>(list_id_dist_Base_ennemie_hero[j]) = buffer; // Save ancien id // Save new id  
                                buffer = save; // nouvelle id pour prochain tour 

                                save_3 = get<1>(list_id_dist_Base_ennemie_hero[j]); // Save dist ancien
                                get<1>(list_id_dist_Base_ennemie_hero[j]) = buffer_2; // Save new id  
                                buffer_2 = save_3; // nouvelle dist pour prochain tour 
                            }
                
                            save_2 = Save_Min_dist[j]; // Save min actuel 
                            Save_Min_dist[j] = dist_1; // Save new min 
                            dist_1 = save_2; // Save new dist avec celle qu on descend  
                        }
                    }
                }

                // hero Ennemie
                if(entite.List_hero_Ennemie.size() !=0)
                {
                    save = 0;
                    save_2 = 0;
                    save_3 = 0;
                    compteur = 3;

                    Save_Min_dist.clear();
                    Save_Min_dist.resize(entite.List_hero_Ennemie.size());

                    for(auto &i : Save_Min_dist){i = i + 10000000;}

                    for(int i = 0; i<entite.List_hero_Ennemie.size(); i++)
                    {
                        double dist_1 = sqrt(pow(base.List_Base[k].base_x - entite.List_hero_Ennemie[i].x, 2) + pow(base.List_Base[k].base_y - entite.List_hero_Ennemie[i].y, 2));
                        int buffer = i; //entite.List_monstre[i].id
                        int buffer_2 = dist_1;

                        for(int j = 0; j<Save_Min_dist.size(); j++)
                        {
                            if(dist_1 < Save_Min_dist[j])
                            {
                                if(k == 0)
                                {
                                    save = get<0>(list_id_dist_Base_allie_hero[compteur]);
                                    get<0>(list_id_dist_Base_allie_hero[compteur]) = buffer; // Save ancien id // Save new id  
                                    buffer = save; // nouvelle id pour prochain tour 

                                    save_3 = get<1>(list_id_dist_Base_allie_hero[compteur]); // Save ancien dist
                                    get<1>(list_id_dist_Base_allie_hero[compteur]) = buffer_2; // Save new dist  
                                    buffer_2 = save_3; // nouvelle dist pour prochain tour 
                                }
                                else
                                {
                                    save = get<0>(list_id_dist_Base_ennemie_hero[compteur]);
                                    get<0>(list_id_dist_Base_ennemie_hero[compteur]) = buffer; // Save ancien id // Save new id  
                                    buffer = save; // nouvelle id pour prochain tour 

                                    save_3 = get<1>(list_id_dist_Base_ennemie_hero[compteur]); // Save ancien dist
                                    get<1>(list_id_dist_Base_ennemie_hero[compteur]) = buffer_2; // Save new dist  
                                    buffer_2 = save_3; // nouvelle dist pour prochain tour 
                                }
                                
                                save_2 = Save_Min_dist[j]; // Save min actuel 
                                Save_Min_dist[j] = dist_1; // Save new min 
                                dist_1 = save_2; // Save new dist avec celle qu on descend  
                            }

                            compteur++;
                        }

                        compteur = 3;
                    }
                }
            }      
        }
        
        /*
        * Fonction qui va donner la distance de mes hero(allie) range dans l 'ordre croissant aux monstre, hero allie(lui meme) et hero opposent
        * Toutes ces donnes stocke dans des tableau(id a l'interieure correspond aux id d origine aux tableau pour ganger du temps)
        * @param rien
        * return rien
        */
        void distance_hero_entity()
        {
            for(auto e = 0; e<3; e++)// Parcours 3 heros list_id_dist_hero_M_H_HO
            {
                //Distance monstre--------------------------------------------------
                int save = 0;
                double save_2 = 0;
                int save_3 = 0;
                int compteur = 0;

                //Vector de tuple provisoire
                vector<tuple<int,double>> valeur_P;
                valeur_P.resize(entite.List_monstre.size());
                //---

                vector<double> Save_Min_dist;
                Save_Min_dist.resize(entite.List_monstre.size());

                for(auto &i : Save_Min_dist){i = i + 10000000;}

                for(int i = 0; i<entite.List_monstre.size(); i++)
                {
                    double dist_1 = sqrt(pow(entite.List_hero_Allie[e].x - entite.List_monstre[i].x, 2) + pow(entite.List_hero_Allie[e].y - entite.List_monstre[i].y, 2));
                    int buffer = i; //entite.List_monstre[i].id
                    int buffer_2 = dist_1;

                    for(int j = 0; j<Save_Min_dist.size(); j++)
                    {
                        if(dist_1 < Save_Min_dist[j])
                        {
                            save = get<0>(valeur_P[j]); // Save ancien id
                            get<0>(valeur_P[j]) = buffer;  // Save new id  
                            buffer = save; // nouvelle id pour prochain tour 
                            
                            save_3 = get<1>(valeur_P[j]); // Save ancien dist
                            get<1>(valeur_P[j]) = buffer_2;  // Save new dist  
                            buffer_2 = save_3; // nouvelle dist pour prochain tour 

                            save_2 = Save_Min_dist[j]; // Save min actuel 
                            Save_Min_dist[j] = dist_1; // Save new min 
                            dist_1 = save_2; // Save new dist avec celle qu on descend  
                        } 
                    }
                }

                //Ajout valeur hero 1,2,3 dans le tableau
                list_id_dist_hero_M.push_back(valeur_P); // Tableau contenant la distance et l index(case array origine) hero -> monstre

                //Distance hero----------------------------------------------------------
                save = 0;
                save_2 = 0;
                save_3 = 0;
                compteur = 0;

                //Vector de tuple provisoire
                valeur_P.resize(entite.List_hero_Allie.size() - 1);
                //----

                Save_Min_dist.resize(entite.List_hero_Allie.size() - 1);

                for(auto &i : Save_Min_dist){i = i + 10000000;}

                for(int i = 0; i<entite.List_hero_Allie.size(); i++)
                {
                    if(e != i)
                    {
                        double dist_1 = sqrt(pow(entite.List_hero_Allie[e].x - entite.List_hero_Allie[i].x, 2) + pow(entite.List_hero_Allie[e].y - entite.List_hero_Allie[i].y, 2));
                        int buffer = i; //entite.List_monstre[i].id
                        int buffer_2 = dist_1;

                        for(int j = 0; j<Save_Min_dist.size(); j++)
                        {
                            if(dist_1 < Save_Min_dist[j])
                            {
                                save = get<0>(valeur_P[j]); // Save ancien id
                                get<0>(valeur_P[j]) = buffer;  // Save new id  
                                buffer = save; // nouvelle id pour prochain tour 
                                
                                save_3 = get<1>(valeur_P[j]); // Save ancien dist
                                get<1>(valeur_P[j]) = buffer_2;  // Save new dist  
                                buffer_2 = save_3; // nouvelle dist pour prochain tour 

                                save_2 = Save_Min_dist[j]; // Save min actuel 
                                Save_Min_dist[j] = dist_1; // Save new min 
                                dist_1 = save_2; // Save new dist avec celle qu on descend  
                            } 
                        }
                    }
                }

                //Ajout valeur hero 1,2,3 dans le tableau
                list_id_dist_hero_H.push_back(valeur_P); // Tableau contenant la distance et l index(case array origine) hero -> hero allie


                //Distance hero_opposent-------------------------------------------------
                save = 0;
                save_2 = 0;
                save_3 = 0;
                compteur = 0;

                //Vector de tuple provisoire
                valeur_P.resize(entite.List_hero_Ennemie.size());
                //----

                Save_Min_dist.resize(entite.List_hero_Ennemie.size());

                for(auto &i : Save_Min_dist){i = i + 10000000;}

                for(int i = 0; i<entite.List_hero_Ennemie.size(); i++)
                {
                    double dist_1 = sqrt(pow(entite.List_hero_Allie[e].x - entite.List_hero_Ennemie[i].x, 2) + pow(entite.List_hero_Allie[e].y - entite.List_hero_Ennemie[i].y, 2));
                    int buffer = i; //entite.List_monstre[i].id
                    int buffer_2 = dist_1;

                    for(int j = 0; j<Save_Min_dist.size(); j++)
                    {
                        if(dist_1 < Save_Min_dist[j])
                        {
                            save = get<0>(valeur_P[j]); // Save ancien id
                            get<0>(valeur_P[j]) = buffer;  // Save new id  
                            buffer = save; // nouvelle id pour prochain tour 
                            
                            save_3 = get<1>(valeur_P[j]); // Save ancien dist
                            get<1>(valeur_P[j]) = buffer_2;  // Save new dist  
                            buffer_2 = save_3; // nouvelle dist pour prochain tour 

                            save_2 = Save_Min_dist[j]; // Save min actuel 
                            Save_Min_dist[j] = dist_1; // Save new min 
                            dist_1 = save_2; // Save new dist avec celle qu on descend  
                        } 
                    }
                }

                //Ajout valeur hero 1,2,3 dans le tableau
                list_id_dist_hero_HO.push_back(valeur_P); // Tableau contenant la distance et l index(case array origine) hero -> hero opposent
            }
        }
        
        /*
        * Donne la distance range dans l ordre croissant des monstre connuent autour des hero opposant
        * @param aucun
        * return rien
        */
        void id_dist_heroopposent_monstre()
        {
            for(int e = 0; e<entite.List_hero_Ennemie.size(); e++)
            {
                //Distance monstre--------------------------------------------------
                int save = 0;
                double save_2 = 0;
                int save_3 = 0;
                int compteur = 0;

                //Vector de tuple provisoire
                vector<tuple<int,double>> valeur_P;
                valeur_P.resize(entite.List_monstre.size());
                //---

                vector<double> Save_Min_dist;
                Save_Min_dist.resize(entite.List_monstre.size());

                for(auto &i : Save_Min_dist){i = i + 10000000;}

                for(int i = 0; i<entite.List_monstre.size(); i++)
                {
                    double dist_1 = sqrt(pow(entite.List_hero_Ennemie[e].x - entite.List_monstre[i].x, 2) + pow(entite.List_hero_Ennemie[e].y - entite.List_monstre[i].y, 2));
                    int buffer = i; //entite.List_monstre[i].id
                    int buffer_2 = dist_1;

                    for(int j = 0; j<Save_Min_dist.size(); j++)
                    {
                        if(dist_1 < Save_Min_dist[j])
                        {
                            save = get<0>(valeur_P[j]); // Save ancien id
                            get<0>(valeur_P[j]) = buffer;  // Save new id  
                            buffer = save; // nouvelle id pour prochain tour 
                            
                            save_3 = get<1>(valeur_P[j]); // Save ancien dist
                            get<1>(valeur_P[j]) = buffer_2;  // Save new dist  
                            buffer_2 = save_3; // nouvelle dist pour prochain tour 

                            save_2 = Save_Min_dist[j]; // Save min actuel 
                            Save_Min_dist[j] = dist_1; // Save new min 
                            dist_1 = save_2; // Save new dist avec celle qu on descend  
                        } 
                    }
                }

                //Ajout monstre par rapport a chaque hero ennemie
                list_id_distance_hero_opposent_monstre.push_back(valeur_P);
            }
        }            
};

int main()
{
    IA ia;
    ia.loop_game();
}
