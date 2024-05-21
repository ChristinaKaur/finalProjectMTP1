// Entrada: Es demana l'any actual juntament amb el nombre d'habitacions que te l'hotel (individuals i dobles) amb el preu d'aquestes. Tambe es demana el nom del fitxer de reserves de l'hotel.
// Sortida: Es mostra les reserves, per ordre de lectura, que es poden fer, que no es poden fer, que s'han de reduir, que s'han d'ampliar o reservar per l'any vinent. Sequidament es mostren les reserces actives de l'any actual ordenardes per DNI juntament amb el menú d'opcions del programa.

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>

using namespace std;

struct Data {
	int dia;
	int mes;
	int any;
	int dia_any;
};
struct Ocupacio {
	int individual;
	int doble;
};
struct Reserva{
	string nom;
	string dni;
	string telf;
	Data inici;
	int nits;
	Ocupacio hab;
	double preu;
	double pagat;
};

const int MAX = 100;
typedef Reserva Vector_reserves[MAX];
struct Reserves {
	Vector_reserves vec;
	int n_hab_individual, n_hab_doble;
	double preu_hab_individual, preu_hab_doble;
	int n;
};

const int MAX_DIA = 31, MAX_MES = 12;
typedef Ocupacio Vector_ocupacio[MAX_DIA+1][MAX_MES+1]; //index: 0..31, 0..12. NO l'utilitzem
const int DIES_MES[13] = {0, 31,28,31,30,31,30,31,31,30,31,30,31};

const string MODIFICACIO_FETA = "Modificacio realitzada";
const string RESERVA_FETA = "Reservat: ";

void mostrar_reserva(const Reserva& r)
{
	// Pre: r ha d'estar omplert correctament.
	// Post: es mostra per pantalla la reserva r. 
	cout << "- " << left << setfill(' ') << setw(10) << r.dni << " " << right << setw(15) << r.nom << " - " << setw(12) << r.telf << ", " << setw(2) << r.inici.dia << "/ " << setw(2) << r.inici.mes << " - " << setw(2) << r.nits << "n (" << setw(2) << r.hab.doble << "d, " << setw(2) << r.hab.individual << "i) - " << setw(7) << r.preu << " eur (" << setw(7) << r.pagat << " pagats)" << endl;
}

void obrir_fitxer_amb_reserves(ifstream& f_in)
{
	// Pre: --
	// Post: retorna un fitxer obert.
	string nom_fitxer;
	cout << "Nom del fitxer amb les reserves existents: ";
	cin >> nom_fitxer;

	f_in.open(nom_fitxer.c_str());
	while (not f_in.is_open())
	{
		f_in.clear();
		cout << "Error a l'obrir el fitxer" << endl;
		cout << "Introdueix un altre nom de fitxer: ";
		cin >> nom_fitxer;
		f_in.open(nom_fitxer.c_str());
	}
}

Reserva llegir_reserva(ifstream& f_in)
{
	// Pre: f_in ha de ser un fitxer obert.
	// Post: retorna una reserva llegida del fitxer.
	Reserva r;
	f_in >> r.nom;
	if (!f_in.eof())
	{
		f_in >> r.dni >> r.telf >> r.inici.dia >> r.inici.mes >> r.inici.any >> r.nits >> r.hab.individual >> r.hab.doble;
	}
	else
		f_in.close();
	return r;
}

void guardar_any_vinent(ofstream& f_out, Reserva& r)
{
	// Pre: f_out ha de ser un fitxer obert correctament, r ha de tenir tots els camps omplerts correctament.
	// Post: guarda r a f_out.

	if (f_out.is_open()) //per evitar errors
		f_out << r.nom << " " << r.dni << " " << r.telf << " " << r.inici.dia << " " << r.inici.mes << " " << r.inici.any << " " << r.nits << " " << r.hab.individual << " " << r.hab.doble << endl;
	else
		cout << "No es pot escriure al fitxer de sortida. No s'ha obert correctament" << endl;
}

bool es_any_traspas(const int any)
{
	// Pre: --
	// Post: retorna true si un any es de traspas, false altrament.
	return ((any % 400 == 0) or (any % 4 == 0 and any % 100 != 0));
}

int es_feb_traspas(const Data data)
{
	// Pre: --
	// Post: retorna 1 si data.any es un any de traspas i data.mes es 2.
	int feb = 0;
	if (es_any_traspas(data.any) and data.mes == 2) feb = 1;
	return feb;
}

bool hi_ha_habitacions_lliures(const Reserves& reserves, const Vector_ocupacio& o, const Reserva& r, bool& es_doble, int& nit)
{
	// Pre: el vector o i la taula r han d'estar omplerts correctament.
	// Post: retorna true si hi ha habitacions lliures, false altrament. es_doble retorna true si l'habitacio que falta és doble, false si és individual i la nit que falta.
	bool doble = r.hab.doble > reserves.n_hab_doble, individual = r.hab.individual > reserves.n_hab_individual;
	if (doble or individual) nit = 0;
	else nit = -1;
	while ((!doble and !individual) and nit + 1 < r.nits)
	{
		int feb = es_feb_traspas(r.inici);
		if (r.inici.dia + nit + 1 <= DIES_MES[r.inici.mes] + feb)
		{
			doble = o[r.inici.dia + nit + 1][r.inici.mes].doble + r.hab.doble > reserves.n_hab_doble;
			individual = o[r.inici.dia + nit + 1][r.inici.mes].individual + r.hab.individual > reserves.n_hab_individual;
		}
		else if (r.inici.dia + nit + 1 > DIES_MES[r.inici.mes] + feb)
		{
			doble = o[DIES_MES[r.inici.mes] + feb - r.inici.dia + nit + 1][r.inici.mes + 1].doble + r.hab.doble > reserves.n_hab_doble;
			individual = o[DIES_MES[r.inici.mes] + feb - r.inici.dia + nit + 1][r.inici.mes + 1].individual + r.hab.individual > reserves.n_hab_individual;
		}
		nit++;
	}
	
	es_doble = false;
	if (doble) es_doble = true;

	return (!doble and !individual);
}

bool data_igual(const Data& a, const Data& b, const int& nits)
{
	// Pre: a i b han d'estar omplerts correctament.
	// Post: retorna true si a més nits es igual a b, false altrament.
	int feb = es_feb_traspas(a);
	return ((a.dia + nits == b.dia and a.mes == b.mes) or (a.dia + nits > DIES_MES[a.mes] + feb and (DIES_MES[a.mes] + feb - a.dia + nits == b.dia and a.mes + 1 == b.mes)));
}

bool data_superior(const Data& a, const Data& b, const int& nits)
{
	// Pre: les dates a i b han d'estar omplerts correctament.
	// Post: retorna true si la data a més nits es superior a la data b, false altrament.
	int feb = es_feb_traspas(a);
	return (((a.dia + nits <= DIES_MES[a.mes] + feb) and (((a.mes == b.mes) and (a.dia + nits > b.dia)) or (a.mes > b.mes))) or (((a.mes + 1 == b.mes) and (a.dia + nits - DIES_MES[a.mes] + feb > b.dia)) or (a.mes + 1 > b.mes)));
}

bool data_inferior(const Data& a, const Data& b)
{
	// Pre: les dates a i b han d'estar omplerts correctament.
	// Post: retorna true si la data a es inferior a la data b, false altrament.
	return (((a.mes == b.mes) and (a.dia < b.dia)) or (a.mes < b.mes));
}

bool reserva_igual(const Reserva& a, const Reserva& b)
{
	// Pre: --
	// Post: retorna true si el dni, la data d'inici i les nits de la reserva a son iguals a les de la b, false altrament.
	return a.dni == b.dni and a.inici.dia == b.inici.dia and a.inici.mes == b.inici.mes and a.nits == b.nits;
}

bool reserva_superior(const Reserva& a, const Reserva& b)
{
	// Pre: --
	// Post: retorna true si el dni, la data d'inici i les nits de la reserva a son superiors a les de la b, false altrament.
	/*
	if (a.dni == b.dni)
	{
		if (a.inici.mes == b.inici.mes)
		{
			if (a.inici.dia == b.inici.dia)
			{
				if (a.nits > b.nits)
					mes_gran = true;																			
			}
			else if (a.inici.dia > b.inici.dia)
				mes_gran = true;
		}
		else if (a.inici.mes > b.inici.mes)
			mes_gran = true;
	}
	else if (a.dni > b.dni)
		mes_gran = true;
	*/

	return (a.dni > b.dni) or ((a.dni == b.dni) and (data_superior(a.inici, b.inici, 0) and (a.nits > b.nits)));
}

void eliminar_de(Reserves& taula, const int& pos)
{
	// Pre: 0<=taula.n<MAX i taula[0..taula.n-1] està ordenat i taula=TAULA i taula.n=TAULA.N i 0<=pos<=taula.n
	// Post: taula.n=TAULA.N+1, taula[0..taula.n-1] està ordenat per dni, inici i nits i és el resultat d'eliminar r de TAULA[0..TAULA.N-1]
	int i = pos;
	while (i < taula.n)
	{
		taula.vec[i] = taula.vec[i + 1];
		i++;
	}
	taula.n--;
}

bool existeix(const Reserves& reserves, const Reserva& r, int& pos)
{
	// Pre: 0 <= reserves.n <= MAX i reserves[0..reserves.n-1] esta ordenat i r est'a omplerts correctament.
	// Post: si r existeix a reserves[0..reserves.n-1], trobat es true i pos es la seva posició, altrament trobat és false i pos és la posició d'inserció d'element a reserves en el rang 0..reserves.n.
	bool trobat = false;
	int dreta = reserves.n - 1, esq = 0, mig = 0;

	while (esq <= dreta and !trobat)
	{
		mig = (esq + dreta) / 2;
		if (reserva_igual(reserves.vec[mig], r)) trobat = true;
		else if (reserva_superior(reserves.vec[mig], r)) dreta = mig - 1;
		else esq = mig + 1;
	}

	if (trobat) pos = mig;
	else pos = esq;

	return trobat;
}

void amplia_reserva(Reserva& reserva_feta, const Reserva& nova)
{
	// Pre: la reserva nova ha d'estar omplerta correctament.
	// Post: es retorna reserva_feta amb les habitacions i el preu de la reserva nova afegides.
	reserva_feta.hab.individual += nova.hab.individual;
	reserva_feta.hab.doble += nova.hab.doble;
	reserva_feta.preu += nova.preu;
}

void reduir_nits(Reserva& r)
{
	// Pre: la reserva r ha d'estar omplerta correctament.
	// Post: es retorna r amb les nits reduides.
	int feb = es_feb_traspas(r.inici);
	r.nits = DIES_MES[r.inici.mes] + feb - r.inici.dia + 1;
	cout << "Reduim a " << r.nits << " nits ";
}

void afegir_a(Reserves& taula, const Reserva& r, const int& pos)
{
	// Pre: 0<=taula.n<MAX i taula[0..taula.n-1] està ordenat i taula=TAULA i taula.n=TAULA.N i 0<=pos<=taula.n, r esta omplerta correctament.
	// Post: taula.n=TAULA.N+1, taula[0..taula.n-1] està ordenat per dni, inici i nits i és el resultat d'inserir r a TAULA[0..TAULA.N-1] a pos.
	int i = taula.n - 1;

	while (i >= pos)
	{
		taula.vec[i + 1] = taula.vec[i];
		i--;
	}

	taula.vec[pos] = r;
	taula.n++;
}

void mostrar_titol(string titol)
{
	// Pre: --
	// Post: es mostra per pantalla titol.
	cout << setw(20) << setfill('_') << "_" << " " << setw(22) << setfill(' ') << left << titol << " " << setw(20) << setfill('_') << "_" << endl << endl;
}

bool es_any_seguent(const Reserva& r)
{
	// Pre: r ha d'estar omplert correctament.
	// Post: es retorna true si r.inici mes r.nits pertany a l'any seguent.
	return (r.inici.dia + r.nits > DIES_MES[12]) and (r.inici.mes + 1 > MAX_MES);
}

double calcular_preu(const Reserves& reserves, const Reserva& r)
{
	// Pre: reserves i r han d'estar omplerts correctament.
	// Post: retorna el preu de la reserva r.
	return ((reserves.preu_hab_individual * r.hab.individual) + (reserves.preu_hab_doble * r.hab.doble)) * r.nits;
}

void amplia_ocupacio(const Reserva r, Vector_ocupacio& o)
{
	// Pre: la reserva r ha d'estar omplerta correctament.
	// Post: s'omple la taula d'ocupacio o amb les habitacions que hi ha a la reserva r en les nits que les te reservades.
	for (int i = 0; i < r.nits; i++)
	{
		int feb = es_feb_traspas(r.inici);
		if (r.inici.dia + i <= DIES_MES[r.inici.mes] + feb)
		{
			o[r.inici.dia + i][r.inici.mes].doble += r.hab.doble;
			o[r.inici.dia + i][r.inici.mes].individual += r.hab.individual;
		}
		else
		{
			o[DIES_MES[r.inici.mes] + feb - r.inici.dia + i][r.inici.mes + 1].doble += r.hab.doble;
			o[DIES_MES[r.inici.mes] + feb - r.inici.dia + i][r.inici.mes + 1].individual += r.hab.individual;
		}
	}
}

void no_hi_ha_habitacions(const bool es_doble, const Reserva r, const int& n_nit)
{
	// Pre: --
	// Post: es mostra per pantalla que no hi ha habitacions individulas/dobles lliures la nit n_nit.
	Data dia_sense_habitacions = r.inici;
	int feb = es_feb_traspas(r.inici);
	if (r.inici.dia + n_nit <= DIES_MES[r.inici.mes] + feb)
	{
		dia_sense_habitacions.dia = r.inici.dia + n_nit;
	}
	else
	{
		dia_sense_habitacions.dia = r.inici.dia + n_nit - DIES_MES[r.inici.mes] + feb;
		dia_sense_habitacions.mes++;
	}

	cout << "No hi ha ";
	if (es_doble) cout << r.hab.doble << " habitacions dobles";
	else cout << r.hab.individual << " habitacions individuals";
	cout << " la nit " << n_nit+1 << " " << dia_sense_habitacions.dia << "/" << dia_sense_habitacions.mes << " No s'ha realitzat la reserva" << endl;
}

void processar_reserva_fitxer(Reserves& reserves_fetes, Vector_ocupacio& ocupacio, Reserva& r, int any, ofstream& f_out)
{
	// Pre: la reserva r ha d'estar omplerta i f_out ha d'estar obert correctament.
	// Post: si hi ha habitacions lliures en l'hotel i r no existeix a reserves_fetes la guarda, altrament s'amplia r i es guarda, també es guarden les habitacions reservades al vector ocupacio, anmenys que r sigui una reserva de l'any seguent, si es el cas es guarda r a f_out.
	if (any >= r.inici.any)
	{
		bool es_doble = false;
		int n_nit = 0;
		if (hi_ha_habitacions_lliures(reserves_fetes, ocupacio, r, es_doble, n_nit))
		{
			if (es_any_seguent(r))
				reduir_nits(r);

			r.preu = calcular_preu(reserves_fetes, r);
			r.pagat = 0;

			int pos = 0;
			if (existeix(reserves_fetes, r, pos))
			{
				amplia_reserva(reserves_fetes.vec[pos], r);
				amplia_ocupacio(reserves_fetes.vec[pos], ocupacio);
				cout << "Reserva ampliada a " << reserves_fetes.vec[pos].hab.individual << "i i " << reserves_fetes.vec[pos].hab.doble << "d - preu: " << reserves_fetes.vec[pos].preu << " eur." << endl;

				cout << RESERVA_FETA;
				mostrar_reserva(reserves_fetes.vec[pos]);
			}
			else if (reserves_fetes.n < MAX)
			{
				afegir_a(reserves_fetes, r, pos);
				amplia_ocupacio(reserves_fetes.vec[pos], ocupacio);

				cout << RESERVA_FETA;
				mostrar_reserva(reserves_fetes.vec[pos]);
			}
			else cout << "No es poden fer mes reserves " << endl;
		}
		else no_hi_ha_habitacions(es_doble, r, n_nit);
	}
	else
	{
		r.preu = calcular_preu(reserves_fetes, r);
		r.pagat = 0;
		guardar_any_vinent(f_out, r);
	}
}

void processar_reserves_fitxer(Reserves& reserves_fetes, Vector_ocupacio& ocupacio, int any, ofstream& f_out)
{
	// Pre: el fitxer f_out ha d'estar obert correctament.
	// Post: es llegeixen totes les reserves d'un fitxer i es processen.
	ifstream f_in;
	obrir_fitxer_amb_reserves(f_in);

	Reserva r = llegir_reserva(f_in);
	while (not f_in.eof())
	{
		processar_reserva_fitxer(reserves_fetes, ocupacio, r, any, f_out);
		r = llegir_reserva(f_in);
	}
	f_in.close();
}

Reserves llegir_habitacions_preus() 
{
	// Pre: --
	// Post: s'entren per teclat el nombre d'habitacions dobles i individuals de l'hotel i els seus preus i es guarden a reserves_fetes.
	Reserves reserves_fetes;
	reserves_fetes.n = 0;
	cout << "Nombre d'habitacions individuals: ";
	cin >> reserves_fetes.n_hab_individual;
	cout << "Nombre d'habitacions dobles: ";
	cin >> reserves_fetes.n_hab_doble;

	cout.setf(ios::fixed);
	cout.precision(2);
	cout << "Preu de les habitacions individuals: ";
	cin >> reserves_fetes.preu_hab_individual;
	cout << "Preu de les habitacions dobles: ";
	cin >> reserves_fetes.preu_hab_doble;
	return reserves_fetes;
}

void mostrar_reserves(const Reserves& taula, string text)
{
	// Pre: taula ha d'estar omplerta correctament.
	// Post: es mostren per pantalla totes les reserves de taula.
	mostrar_titol(text);
	for (int i = 0; i < taula.n; i++) mostrar_reserva(taula.vec[i]);
	mostrar_titol("");
}

void mostrar_menu()
{
	// Pre: --
	// Post: es mostra per pantalla el menu del programa.
	mostrar_titol("Menu");
	cout << "d: actualitzar data" << endl;
	cout << "r: fer reserva" << endl;
	cout << "a: anul.lar reserva" << endl;
	cout << "m: modificar reserva" << endl;
	cout << "p: pagar reserva" << endl;
	cout << "e: esborrar completades" << endl;
	cout << "c: consultar reserves" << endl;
	cout << "o: mostrar ordenat" << endl;
	cout << "t: dates top" << endl;
	cout << "h: ajuda" << endl;
	cout << "s: sortir" << endl;
}

char entrar_opcio()
{
	// Pre: --
	// Post: retorna caracter entrat per teclat
	char opcio;
	cin >> opcio;
	return opcio;
}

Reserves crear_taula_acaben_avui(const Data& data, const Reserves& taula)
{
	// Pre: data ha d'estar omplert correctament.
	// Post: retorna una taula de reserves amb les reserves de taula que acaben el dia data. 
	Reserves taula_c;
	taula_c.n = 0;
	for (int i = 0; i < taula.n; i++)
	{
		if (data_igual(taula.vec[i].inici, data, taula.vec[i].nits))
			afegir_a(taula_c, taula.vec[i], taula_c.n);
	}
	return taula_c;
}

bool reserva_pagada(const Reserva& r)
{
	// Pre: r ha d'estar omplerta correctament.
	// Post: es retorna true si la reserva r està pagada.
	return r.preu == r.pagat;
}

void mostrar_reserves_acaben(const Data& data, const Reserves& taula)
{
	// Pre: --
	// Post: mostra per pantalla les reserves de taula que acaben el dia data, tenint en compte les que no estan pagades.
	Reserves r_acaben = crear_taula_acaben_avui(data, taula);

	mostrar_titol("Acaben avui");
	for (int i = 0; i < r_acaben.n; i++)
	{
		if (!reserva_pagada(r_acaben.vec[i])) cout << 'C';
		else cout << " ";

		cout << " " << setw(2);
		mostrar_reserva(r_acaben.vec[i]);
	}
	mostrar_titol("");
}

void mostrar_reserves_comencen(const Data& data, const Reserves& taula)
{
	// Pre: --
	// Post: mostra per pantalla les reserves de taula que comencen el dia data.
	mostrar_titol("Comencen avui");
	for (int i = 0; i < taula.n; i++)
	{
		if (data_igual(taula.vec[i].inici, data, 0))
		{
			cout << "  " << setw(2);
			mostrar_reserva(taula.vec[i]);
		}
	}
	mostrar_titol("");
}

Reserva llegir_reserva()
{
	// Pre: --
	// Post: es retorna una reserva llegida per teclat.
	Reserva r;
	cout << "Dni, nombre de nits i data d'inici:" << endl;
	cin >> r.dni >> r.nits >> r.inici.dia >> r.inici.mes >> r.inici.any;
	return r;
}

void pagar_reserva(Reserva& r)
{
	// Pre: la reserva r ha d'estar omplerta corrcetament.
	// Post: s'entra per teclat el preu que es vol pagar de r.
	double paga_senyal = r.preu + 1;
	while (paga_senyal > r.preu)
	{
		cout << "Paga i senyal (preu " << r.preu << " eur): ";
		cin >> paga_senyal;
	}
	r.pagat = paga_senyal;
}

bool falten_7_dies(const Data& r, const Data& data_a)
{
	// Pre: les dates r i data_a han d'estar omplertes correctament.
	// Post: es retorna true si hi ha 7 dies o mes de r a data_a, false altrament.
	int feb = es_feb_traspas(data_a);
	return (data_a.mes == r.mes and r.dia - data_a.dia >= 7) or (data_a.mes < r.mes and (DIES_MES[data_a.mes] + feb - data_a.dia) + r.dia >= 7);
}

void intercanvia(Reserva& a, Reserva& b)
{
	//Pre: a=A i b=B
	//Post: a=B i b=A
	Reserva aux = a;
	a = b;
	b = aux;
}

void modificar_reserva_existent(Reserves& taula, Vector_ocupacio& ocupacio, Reserva& r, int& pos, const Data& data_a)
{
	// Pre: taula i r ha d'estar omplerta correctament.
	// Post: es modifica la reserva r.
	double pagat = taula.vec[pos].pagat;
	double preu = taula.vec[pos].preu;
	cout << "Nova reserva: ";
	mostrar_reserva(r);

	cout << "Processar reserva (s/n): ";
	char opcio = entrar_opcio();
	if (opcio == 's')
	{
		if (falten_7_dies(taula.vec[pos].inici, data_a))
		{
			intercanvia(taula.vec[pos], r);
			amplia_ocupacio(taula.vec[pos], ocupacio);
			if (pagat == 0)
			{
				cout << MODIFICACIO_FETA << endl;
				pagar_reserva(taula.vec[pos]);

				cout << RESERVA_FETA;
				mostrar_reserva(taula.vec[pos]);
			}
			if (taula.vec[pos].preu <= pagat)
			{
				cout << "Reserva pagada";
				if (taula.vec[pos].preu < pagat)
					cout << ", es retornen " << r.preu - pagat  << "eur." << endl;
				cout << MODIFICACIO_FETA << endl;
			}
		}
		else
		{
			cout << "Es mantindra el preu inicial: " << preu << " eur." << endl;

			cout << "Fem la modificacio (s/n)? ";
			opcio = entrar_opcio();
			if (opcio == 's')
			{
				r.preu = preu;
				intercanvia(taula.vec[pos], r);
				amplia_ocupacio(taula.vec[pos], ocupacio);
				cout << MODIFICACIO_FETA << endl;
			}
		}
	}
	else
	{
		cout << "S'elimina la reserva"; 
		mostrar_reserva(taula.vec[pos]);
		eliminar_de(taula, pos);
	}

}

void processar_reserva(Reserves& taula, Vector_ocupacio& ocupacio, Reserva& r, const Data& data_a, ofstream& f_out)
{
	// Pre: la reserva r ha d'estar omplerta i f_out ha d'estar obert correctament.
	// Post: si hi ha habitacions lliures a l'hotel i r no existeix a taula la guarda, també es guarden les habitacions reservades al vector ocupacio i es demana si es vol efectuar, altrament es modifica la reserva ja existent, anmenys que r sigui una reserva de l'any seguent, si es el cas es guarda r a f_out.
	if (data_a.any >= r.inici.any)
	{
		bool es_doble = false;
		int n_nit = 0;
		if (hi_ha_habitacions_lliures(taula, ocupacio, r, es_doble, n_nit))
		{
			if (es_any_seguent(r))
				reduir_nits(r);

			r.preu = calcular_preu(taula, r);
			r.pagat = 0;

			int pos = 0;
			if (existeix(taula, r, pos))
			{
				cout << "Es procedeix a modificar la reserva existent ";
				mostrar_reserva(taula.vec[pos]);
				modificar_reserva_existent(taula, ocupacio, r, pos, data_a);
			}
			else if (taula.n < MAX)
			{
				afegir_a(taula, r, pos);
				amplia_ocupacio(taula.vec[pos], ocupacio);

				cout << "Reserva actual: ";
				mostrar_reserva(taula.vec[pos]);

				cout << "Efectuar nova reserva (s/n): ";
				char opcio = entrar_opcio();
				if (opcio == 's')
				{
					pagar_reserva(taula.vec[pos]);
					cout << RESERVA_FETA;
					mostrar_reserva(taula.vec[pos]);
				}
			}
			else cout << "No es poden fer mes reserves " << endl;
		}
		else no_hi_ha_habitacions(es_doble, r, n_nit);
	}
	else
	{
		r.preu = calcular_preu(taula, r);
		r.pagat = 0;
		guardar_any_vinent(f_out, r);
	}
}

int posicio_del_dni_minim_des_de(const Reserves& taula, int inici)
{
	//Pre: 0 <= inici < taula.n <= MAX
	//Post: retorna la posició de la reserva amb dni mínim de taula[inici..taula.n - 1]
	int pos_min = inici;
	for (int i = inici + 1; i < taula.n; i++)
	{
		if (reserva_superior(taula.vec[pos_min], taula.vec[i])) pos_min = i;
	}
	return pos_min;
}

void mostrar_ordenat_dni(const Reserves& vec)
{
	//Pre: 0<=vec.n<=MAX, vec=VEC
	//Post: taula[0..taula.n-1] conté el elements de TAULA[0..taula.n-1] ordenats per dni creixent i mostrar taula
	Reserves taula = vec;
	for (int i = 0; i < taula.n - 1; i++) {
		int pos_min = posicio_del_dni_minim_des_de(taula, i);
		intercanvia(taula.vec[i], taula.vec[pos_min]);
	}
	cout << "RESERVES ORDENADES PER DNI:" << endl;
	mostrar_reserves(taula, "Reserves actives");
}

int posicio_del_inici_minim_des_de(const Reserves& taula, int inici)
{
	//Pre: 0 <= inici < taula.n <= MAX
	//Post: retorna la posició de la reserva amb data d'inici mínim de taula[inici..taula.n - 1]
	int pos_min = inici;
	for (int i = inici + 1; i < taula.n; i++)
	{
		//if (!data_superior(taula.vec[i].inici, taula.vec[pos_min].inici, 0)) pos_min = i;
		if (data_inferior(taula.vec[i].inici, taula.vec[pos_min].inici)) pos_min = i;
		else if ((data_igual(taula.vec[i].inici, taula.vec[pos_min].inici, 0)) and (((taula.vec[i].dni == taula.vec[pos_min].dni) and (taula.vec[i].nits < taula.vec[pos_min].nits)) or (taula.vec[i].dni < taula.vec[pos_min].dni))) pos_min = i;
	}
	return pos_min;
}

void mostrar_ordenat_inici(const Reserves& vec)
{
	//Pre: 0<=vec.n<=MAX, vec=VEC
	//Post: taula[0..taula.n-1] conté el elements de TAULA[0..taula.n-1] ordenats per data d'inici decreixent i mostrar taula
	Reserves taula = vec;
	for (int i = 0; i < taula.n - 1; i++) {
		int pos_min = posicio_del_inici_minim_des_de(taula, i);
		intercanvia(taula.vec[i], taula.vec[pos_min]);
	}
	cout << "RESERVES ORDENADES PER DATA D'INICI:" << endl;
	mostrar_reserves(taula, "Reserves actives");
}

int posicio_del_nom_minim_des_de(const Reserves& taula, int inici)
{
	//Pre: 0 <= inici < taula.n <= MAX
	//Post: retorna la posició de la reserva amb nom mínim de vec[inici..taula.n - 1]
	int pos_min = inici;
	for (int i = inici + 1; i < taula.n; i++)
	{
		if ((taula.vec[i].nom < taula.vec[pos_min].nom) or ((taula.vec[i].nom == taula.vec[pos_min].nom) and (reserva_superior(taula.vec[pos_min], taula.vec[i])))) pos_min = i;
	}
	return pos_min;
}

void mostrar_ordenat_nom(const Reserves& vec)
{
	//Pre: 0<=vec.n<=MAX, vec=VEC
	//Post: taula[0..taula.n-1] conté el elements de TAULA[0..taula.n-1] ordenats per nom decreixent i mostrar taula
	Reserves taula = vec;
	for (int i = 0; i < taula.n - 1; i++) {
		int pos_min = posicio_del_nom_minim_des_de(taula, i);
		intercanvia(taula.vec[i], taula.vec[pos_min]);
	}
	cout << "RESERVES ORDENADES PER NOM:" << endl;
	mostrar_reserves(taula, "Reserves actives");
}

int posicio_del_preu_minim_des_de(const Reserves& taula, int inici)
{
	//Pre: 0 <= inici < taula.n <= MAX
	//Post: retorna la posició de la reserva amb preu mínim de taula[inici..taula.n - 1]
	int pos_min = inici;
	for (int i = inici + 1; i < taula.n; i++)
	{
		if ((taula.vec[i].preu > taula.vec[pos_min].preu) or ((taula.vec[i].preu == taula.vec[pos_min].preu) and (reserva_superior(taula.vec[pos_min], taula.vec[i])))) pos_min = i;
	}
	return pos_min;
}

void mostrar_ordenat_preu(const Reserves& vec)
{
	//Pre: 0<=vec.n<=MAX, vec=VEC
	//Post: taula[0..taula.n-1] conté el elements de TAULA[0..taula.n-1] ordenats per preu decreixent i mostrar taula
	Reserves taula = vec;
	for (int i = 0; i < taula.n - 1; i++) {
		int pos_min = posicio_del_preu_minim_des_de(taula, i);
		intercanvia(taula.vec[i], taula.vec[pos_min]);
	}
	cout << "RESERVES ORDENADES PER PREU:" << endl;
	mostrar_reserves(taula, "Reserves actives");
}

Reserves crear_taula_arxivades(const Data& data, Reserves& taula)
{
	// Pre: taula ha d'estar omplerta correctament.
	// Post: es retorna una taula de totes les reserves de taula que ja s'han finalitzat.
	Reserves arxivades;
	arxivades.n = 0;

	int i = 0;
	while (i < taula.n)
	{
		int feb = es_feb_traspas(taula.vec[i].inici);
		if (!data_superior(taula.vec[i].inici, data, taula.vec[i].nits) and !data_igual(taula.vec[i].inici, data, taula.vec[i].nits))
		{
			afegir_a(arxivades, taula.vec[i], arxivades.n);
			eliminar_de(taula, i);
		}
		else i++;
	}
	return arxivades;
}

Reserves crear_taula_vigents_data(const Data& data, const Reserves& taula)
{
	// Pre: --
	// Post: es retorna una taula de totes les reserves de taula que tenen una data en concret.
	Reserves r_vigents; 
	r_vigents.n = 0;

	for (int i = 0; i < taula.n; i++)
	{
		for (int j = 0; j < taula.vec[i].nits; j++)
		{
			if (data_igual(taula.vec[i].inici, data, j))
				afegir_a(r_vigents, taula.vec[i], r_vigents.n);
		}
	}
	return r_vigents;
}

Reserves crear_taula_dni(const string& dni, const Reserves& taula_actives, const Reserves& taula_arxivades)
{
	// Pre: --
	// Post: es retorna una taula de totes les reserves de taula_actives i taula_arxivades que tenen un dni en concret.
	Reserves r_vigents;
	r_vigents.n = 0;

	int i = 0;
	while (i < taula_actives.n or i < taula_arxivades.n)
	{
		if (taula_actives.vec[i].dni == dni)
			afegir_a(r_vigents, taula_actives.vec[i], r_vigents.n);
		if (taula_arxivades.vec[i].dni == dni)
			afegir_a(r_vigents, taula_arxivades.vec[i], r_vigents.n);
		i++;
	}
	return r_vigents;
}

Data llegir_data()
{
	Data data;
	cout << "Any: ";
	cin >> data.any;
	return data;
}

void omplir_matriu(Vector_ocupacio& ocupacio_hotel)
{
	// Pre: --
	// Post: retorna la matriu inisialitzada.
	for (int dia = 1; dia <= MAX_DIA; dia++)
	{
		for (int mes = 1; mes <= MAX_MES; mes++)
		{
			ocupacio_hotel[dia][mes].individual = 0;
			ocupacio_hotel[dia][mes].doble = 0;
		}
	}
}

void actualitzar_data(Reserves& reserves_actives, Data& data_actual)
{
	// Pre: Data actual ha d'estar omplert correctament.
	// Post: L’usuari introdueix el dia d’avui i mostra primer les reserves que acaben avui (remarcant les que cal cobrar) i després les que comencen avui.
	Data data_nova;
	cout << "Introdueix la nova data: ";
	cin >> data_nova.dia >> data_nova.mes >> data_nova.any;

	data_actual = data_nova;

	mostrar_reserves_acaben(data_actual, reserves_actives);
	mostrar_reserves_comencen(data_actual, reserves_actives);
}

void fer_reserva(Reserves& reserves_actives, Vector_ocupacio& ocupacio_hotel, Data& data_actual, ofstream& f_out)
{
	/*Es demana la data d’inici de la reserva, el nombre de nits i el nombre d’habitacions. Si és pel pròxim any, es demanen totes les dades i s’escriu al fitxer corresponent.
	Si és per aquest any, però acaba passat el 31/12, es redueix el nombre de nits per acabar-la el 31/12. Per les reserves de l’any actual es comprova si hi ha una reserva equivalent a aquesta,
	si hi és es mostra i s’anul·la. A continuació es comprova la viabilitat de la nova reserva, si hi ha habitacions disponibles, es mostra el preu de la reserva, i si el client l’accepta,
	es demanen les dades que falten(demanant si vol fer una paga i senyal per avançat) i es guarda la nova reserva. */
	cout << "Nom, DNI, telf, inici reserva, nits, hab. individuals i dobles:" << endl;
	Reserva reserva_nova;
	cin >> reserva_nova.nom >> reserva_nova.dni >> reserva_nova.telf >> reserva_nova.inici.dia >> reserva_nova.inici.mes >> reserva_nova.inici.any >> reserva_nova.nits >> reserva_nova.hab.individual >> reserva_nova.hab.doble;
	processar_reserva(reserves_actives, ocupacio_hotel, reserva_nova, data_actual, f_out);
}

void anular_reserva(Reserves& reserves_actives, Data& data_actual)
{
	/* anul·lar una reserva : Es demana el DNI, inici i nits de la reserva i es troba la reserva, s’anul·la. Si falten set dies o més cal retornar la paga i senyal, però si falten menys de set dies
	es cobrarà la totalitat de l’import. */
	Reserva r;
	cout << "Dni, nombre de nits i data d'inici:" << endl;
	cin >> r.dni >> r.nits >> r.inici.dia >> r.inici.mes >> r.inici.any;

	int pos = 0;
	if (existeix(reserves_actives, r, pos))
	{
		if (falten_7_dies(r.inici, data_actual)) cout << "No cal pagar la reserva, es retornen " << reserves_actives.vec[pos].pagat << "eur" << endl;
		else cout << "Cal pagar la reserva, falten " << reserves_actives.vec[pos].preu << "eur" << endl;
		eliminar_de(reserves_actives, pos);
	}
	else cout << "No s'ha trobat, no es pot anular" << endl;
}

void modificar_reserva(Reserves& reserves_actives, Vector_ocupacio& ocupacio_hotel, Data& data_actual, char& opcio)
{
	/* modificar una reserva : Es demana el DNI i un caràcter indicant què cal modificar : nits(n), inici(i), habitacions(h), tot(t).Es llegeix la informació que cal modificar.
	Si la modificació implica una reducció de preu, es realitzarà el canvi d’import, només, si falten com a mínim set dies per l’inici de la reserva. */
	Reserva reserva = llegir_reserva();

	int pos = 0;
	if (existeix(reserves_actives, reserva, pos))
	{
		cout << "Modificar nits(n), inici(i), habitacions (h), tot (t): ";
		opcio = entrar_opcio();

		reserva = reserves_actives.vec[pos];
		if (opcio == 'n')
		{
			cout << "Nombre de nits: ";
			cin >> reserva.nits;
		}
		else if (opcio == 'i')
		{
			cout << "Inici: ";
			cin >> reserva.inici.dia >> reserva.inici.mes >> reserva.inici.any;
		}
		else if (opcio == 'h')
		{
			cout << "Nombre d'habitacions inidividuals i dobles: ";
			cin >> reserva.hab.individual >> reserva.hab.doble;
		}
		else if (opcio == 't')
		{
			cout << "Inici, nombre de nits, nombre d'habitacions inidividuals i dobles: ";
			cin >> reserva.inici.dia >> reserva.inici.mes >> reserva.inici.any >> reserva.nits >> reserva.hab.individual >> reserva.hab.doble;
		}
		modificar_reserva_existent(reserves_actives, ocupacio_hotel, reserva, pos, data_actual);
	}
	else cout << "No s'ha trobat, no es pot modificar" << endl;
}

void pagar_reserva(Reserves& reserves_actives)
{
	// pagar una reserva: Es demana el DNI, inici i durada de la reserva i, si la reserva existeix, es cobra l’import de la reserva descomptant la paga i senyal.
	Reserva reserva = llegir_reserva();

	int pos = 0;
	if (existeix(reserves_actives, reserva, pos))
	{
		cout << "Cobrar " << reserves_actives.vec[pos].preu - reserves_actives.vec[pos].pagat << "eur." << endl;
		reserves_actives.vec[pos].pagat = reserves_actives.vec[pos].preu;
	}
	else cout << "No s'ha trobat, no es pot pagar" << endl;
}

void arxivar_reserves(Reserves& reserves_actives, Reserves& reserves_arxivades, Data& data_actual)
{
	// arxivar reserves: S’introdueix una data i s’arxiven les reserves completades en aquesta data
	cout << "Data de finalitacio de les reserves a arxivar: ";
	cin >> data_actual.dia >> data_actual.mes >> data_actual.any;

	reserves_arxivades = crear_taula_arxivades(data_actual, reserves_actives);
	mostrar_reserves(reserves_actives, "Reserves actives");
	mostrar_reserves(reserves_arxivades, "Reserves arxivades");
}

void consultar_reserves(Reserves& reserves_actives, Reserves& reserves_arxivades, char& opcio)
{
	/* consultar reserves : S'introdueix un caràcter (v o d) indicant si es vol consultar les reserves vigents el dia de la data introduïda per teclat(mostrant - les ordenades per DNI, i en cas d'empat
	per data d'inici i nombre de nits) o les associades(al llarg de l’any) a un DNI introduït per teclat(mostrant - les ordenades per data, i en cas d'empat per nombre de nits). */
	cout << "Consultar data (v) o dni (d): ";
	opcio = entrar_opcio();

	Reserves vigents;
	vigents.n = 0;

	if (opcio == 'v')
	{
		Data data;
		cout << "Data: ";
		cin >> data.dia >> data.mes >> data.any;
		vigents = crear_taula_vigents_data(data, reserves_actives);
	}
	else if (opcio == 'd')
	{
		string dni;
		cout << "DNI: ";
		cin >> dni;
		vigents = crear_taula_dni(dni, reserves_actives, reserves_arxivades);
	}
	mostrar_reserves(vigents, "Reserves obtingudes");
}

void mostrar_reserves_no_arxivades_per_ordre(Reserves& reserves_actives, char opcio)
{
	/* mostrar les reserves no arxivades per ordre de : (d)DNI creixent, (i)data d’inici, (n)nom de persona, (p)preu decreixent.En cas d’empat es desempata per data, DNI i nombre de nits,
	excepte en el cas del nom de la persona que es desempata per DNI, data i nombre de nits. */
	cout << "Tipus d'ordre d) DNI - i) inici - n) nom - p) preu: ";
	opcio = entrar_opcio();

	if (opcio == 'd')
		mostrar_ordenat_dni(reserves_actives);
	else if (opcio == 'i')
		mostrar_ordenat_inici(reserves_actives);
	else if (opcio == 'n')
		mostrar_ordenat_nom(reserves_actives);
	else if (opcio == 'p')
		mostrar_ordenat_preu(reserves_actives);
	else
		cout << "Opcio invalida" << endl;
}

void dates_top(Vector_ocupacio ocupacio_hotel)
{
	/*dates top : S’introdueix un nombre natural n i es mostren els n dies de l’any amb més ocupació de més a menys ocupació i en cas d’empat per ordre cronològic(les habitacions dobles compten per dos).*/
	int dies;
	cout << "Nombre de dies: ";
	cin >> dies;
}

int main()
{
	Data data_actual = llegir_data();

	Reserves reserves_actives = llegir_habitacions_preus();

	Vector_ocupacio ocupacio_hotel;
	omplir_matriu(ocupacio_hotel);

	ofstream f_out;
	f_out.open("proxim_any.txt");

	processar_reserves_fitxer(reserves_actives, ocupacio_hotel, data_actual.any, f_out);
	mostrar_titol("Fi processament fitxer");

	mostrar_reserves(reserves_actives, "Reserves actives");

	Reserves reserves_arxivades;
	reserves_arxivades.n = 0;

	cout << "Opcio (h: help): ";
	char opcio = entrar_opcio();

	while (opcio != 's')
	{
		if (opcio == 'd') actualitzar_data(reserves_actives, data_actual);
		else if (opcio == 'r') fer_reserva(reserves_actives, ocupacio_hotel, data_actual, f_out);
		else if (opcio == 'a') anular_reserva(reserves_actives, data_actual);
		else if (opcio == 'm') modificar_reserva(reserves_actives, ocupacio_hotel, data_actual, opcio);
		else if (opcio == 'p') pagar_reserva(reserves_actives);
		else if (opcio == 'e') arxivar_reserves(reserves_actives, reserves_arxivades, data_actual);
		else if (opcio == 'c') consultar_reserves(reserves_actives, reserves_arxivades, opcio);
		else if (opcio == 'o') mostrar_reserves_no_arxivades_per_ordre(reserves_actives, opcio);
		else if (opcio == 't') dates_top(ocupacio_hotel);
		else if (opcio == 'h') mostrar_menu();
		else cout << "Opcio invalida" << endl;
		cout << "Opcio (h: help): ";
		opcio = entrar_opcio();
	}
	mostrar_reserves(reserves_actives, "Reserves actives");
	f_out.close();
	return 0;
}