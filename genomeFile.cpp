#include "genome.h"
#include "endianlove.h"
#include <typeinfo>
#include <exception>
#include <iostream>

using namespace std;

geneNote *genomeFile::findNote(uint8 type, uint8 subtype, uint8 which) {
	for (vector<gene *>::iterator x = genes.begin(); x != genes.end(); x++) {
		gene *t = *x;
			if ((uint8)t->type() == type)
				if ((uint8)t->subtype() == subtype)
					if ((uint8)t->note.which == which)
						return &t->note;

	if (typeid(*t) == typeid(organGene))
		for (vector<gene *>::iterator y = ((organGene *)t)->genes.begin(); y != ((organGene *)t)->genes.end(); y++) {
			gene *s = *y;
        		if ((uint8)s->type() == type)
				if ((uint8)s->subtype() == subtype)
					if ((uint8)s->note.which == which)
						return &s->note;
		}
	}

	return 0;
}

void genomeFile::readNotes(istream &s) {
	uint16 gnover = read16(s);

	if (cversion == 3) {
		uint16 nosvnotes = read16(s);

		for (int i = 0; i < nosvnotes; i++) {
			uint16 type = read16(s);
			uint16 subtype = read16(s);
			uint16 which = read16(s);
			uint16 rule = read16(s);

			// TODO: we currently skip all the notes
			for (int i = 0; i < 19; i++) {
				uint16 skip = read16(s);
				uint8 dummy[skip]; s.read((char *)dummy, skip);
			}
    		}

		uint8 ver;

		while ((ver != 0x02) && (!(s.fail() || s.eof())))
			s >> ver;

		if (ver != 0x02) throw genomeException("c3 gno loading broke ... second majic isn't 0x02");
	}

	uint16 noentries = read16(s);

	for (int i = 0; i < noentries; i++) {
		uint16 type = read16(s);
		uint16 subtype = read16(s);
		uint32 which = read32(s);

		geneNote *n = findNote(type, subtype, which);

		if (n == 0) {
			cout << "warning: couldn't find type/subtype/which matching gno file (wrong GNO file?)\n";
			continue;
		}

		uint16 buflen = read16(s);
		char *buffer = new char[buflen];
		s.read(buffer, buflen);
		n->description = buffer;
		buflen = read16(s);
		delete buffer; buffer = new char[buflen];
		n->comments = buffer;
		delete buffer;		
	}
}

void genomeFile::writeNotes(ostream &s) {
	// TODO
}

gene *genomeFile::nextGene(istream &s) {
	uint8 majic[3];
	s.read((char *)majic, 3);
	if (strncmp((char *)majic, "gen", 3) != 0) throw genomeException("bad majic for a gene");

	s >> majic[0];
	if (majic[0] == 'd') return 0;
	if (majic[0] != 'e')
		throw genomeException("bad majic at stage2 for a gene");

	uint8 type, subtype;
	s >> type >> subtype;

	gene *g = 0;
  
	// the switch statement of doom... is there a better way to do this?
	switch (type) {
		case 0:
			switch (subtype) {
				case 0:
					switch (cversion) {
						case 1: g = new oldBrainLobe(cversion); break;
						case 2: g = new oldBrainLobe(cversion); break;
						case 3: g = new c2eBrainLobe(cversion); break;
						default: g = 0; break;
					} break;
				case 1: g = new organGene(cversion, true); break;
				case 2: g = new c2eBrainTract(cversion); break;
			} break;
		case 1:
			switch (subtype) {
				case 0: g = new bioReceptor(cversion); break;
				case 1: g = new bioEmitter(cversion); break;
				case 2: g = new bioReaction(cversion); break;
				case 3: g = new bioHalfLives(cversion); break;
				case 4: g = new bioInitialConcentration(cversion); break;
				case 5: g = new bioNeuroEmitter(cversion); break;
			} break;
		case 2:
			switch (subtype) {
				case 0: g = new creatureStimulus(cversion); break;
				case 1: g = new creatureGenus(cversion); break;
				case 2: g = new creatureAppearance(cversion); break;
				case 3: g = new creaturePose(cversion); break;
				case 4: g = new creatureGait(cversion); break;
				case 5: g = new creatureInstinct(cversion); break;
				case 6: g = new creaturePigment(cversion); break;
				case 7: g = new creaturePigmentBleed(cversion); break;
				case 8: g = new creatureFacialExpression(cversion); break;
			} break;
		case 3:
			switch (subtype) {
				case 0: g = new organGene(cversion, false); break;
			} break;
	}

	if (g == 0) throw genomeException("genefactory failed");

	if (((typeid(*g) == typeid(bioReaction))
		|| (typeid(*g) == typeid(bioEmitter)))
		|| (typeid(*g) == typeid(bioReceptor))) {
		if (currorgan == 0) {
        		if (cversion == 1) genes.push_back(g); // Creatures 1 doesn't have organs
        		else throw genomeException("reaction/emitter/receptor without an attached organ");
		} else currorgan->genes.push_back(g);
	} else {
		genes.push_back(g);
		if (typeid(*g) == typeid(organGene))
			if (!((organGene *)g)->isBrain())
				currorgan = (organGene *)g;
	}

	s >> *g;

	return g;
}

istream &operator >> (istream &s, genomeFile &f) {
	char majic[3]; s.read(majic, 3);
	if (strncmp((char *)majic, "dna", 3) != 0) throw genomeException("bad majic for genome");

	s >> majic[0];
	f.cversion = majic[0] - 48; // 48 = ASCII '0'
	if ((f.cversion < 1) || (f.cversion > 3)) throw genomeException("unsupported genome version in majic");

	cout << "creaturesGenomeFile: reading genome of version " << (unsigned int)f.cversion << ".\n";
	f.currorgan = 0;
	while (f.nextGene(s) != 0);
	f.currorgan = 0;

	return s;
}

ostream &operator << (ostream &s, const genomeFile &f) {
	s << "dna" << char(f.cversion + 48); // 48 = ASCII '0'

	// iterate through genes
	for (vector<gene *>::iterator x = ((genomeFile &)f).genes.begin(); x != ((genomeFile &)f).genes.end(); x++)
		s << **x;

	s << "gend";

	return s;
}

uint8 geneFlags::operator () () const {
	return ((_mutable?1:0) + (dupable?2:0) + (delable?4:0) + (maleonly?8:0) +
		(femaleonly?16:0) + (notexpressed?32:0) + (reserved1?64:0) + (reserved2?128:0));
}

void geneFlags::operator () (uint8 f) {
	_mutable = ((f & 1) != 0);
	dupable = ((f & 2) != 0);
	delable = ((f & 4) != 0);
	maleonly = ((f & 8) != 0);
	femaleonly = ((f & 16) != 0);
	notexpressed = ((f & 32) != 0);
	reserved1 = ((f & 64) != 0);
	reserved2 = ((f & 128) != 0);
}

ostream &operator << (ostream &s, const gene &g) {
	s << "gene" << g.type() << g.subtype();

	s << g.note.which << g.header.generation << uint8(g.header.switchontime) << g.header.flags();
	if (g.cversion > 1) s << g.header.mutweighting;
	if (g.cversion == 3) s << g.header.variant;

	g.write(s);

	return s;
}

istream &operator >> (istream &s, gene &g) {
	uint8 b;
	s >> g.note.which >> g.header.generation >> b;
	g.header.switchontime = (lifestage)b;
	s >> b;
	g.header.flags(b);
	if (g.cversion > 1) s >> g.header.mutweighting;
	if (g.cversion == 3) s >> g.header.variant;

	g.read(s);

	return s;
}

void bioEmitter::write(ostream &s) const {
	s << organ << tissue << locus << chemical << threshold << rate << gain;
	uint8 flags = (clear?1:0) + (invert?2:0);
	s << flags;
}

void bioEmitter::read(istream &s) {
	s >> organ >> tissue >> locus >> chemical >> threshold >> rate >> gain;
	uint8 flags;
	s >> flags;
	clear = ((flags & 1) != 0);
	invert = ((flags & 2) != 0);
}

void bioHalfLives::write(ostream &s) const {
	for (int i = 0; i < 256; i++) {
	s << halflives[i];
	}
}

void bioHalfLives::read(istream &s) {
	for (int i = 0; i < 256; i++) {
		s >> halflives[i];
	}
}

void bioInitialConcentration::write(ostream &s) const {
	s << chemical << quantity;
}

void bioInitialConcentration::read(istream &s) {
	s >> chemical >> quantity;
}

void bioNeuroEmitter::write(ostream &s) const {
	for (int i = 0; i < 3; i++) {
		s << lobes[i] << neurons[i];
	}
	s << rate;
	for (int i = 0; i < 4; i++) {
		s << chemical[i] << quantity[i];
	}
}

void bioNeuroEmitter::read(istream &s) {
	for (int i = 0; i < 3; i++) {
		s >> lobes[i] >> neurons[i];
	}
	s >> rate;
	for (int i = 0; i < 4; i++) {
		s >> chemical[i] >> quantity[i];
	}
}

void bioReaction::write(ostream &s) const {
	for (int i = 0; i < 4; i++) {
		s << reactant[i];
		s << quantity[i];
	}
	
	s << rate;
}

void bioReaction::read(istream &s) {
	for (int i = 0; i < 4; i++) {
		s >> reactant[i];
		s >> quantity[i];
	}
	
	s >> rate;
}

void bioReceptor::write(ostream &s) const {
	s << organ << tissue << locus << chemical << threshold << nominal << gain;
	uint8 flags = (inverted?1:0) + (digital?2:0);
	s << flags;
}

void bioReceptor::read(istream &s) {
	s >> organ >> tissue >> locus >> chemical >> threshold >> nominal >> gain;
	uint8 flags;
	s >> flags;
	inverted = ((flags & 1) != 0);
	digital = ((flags & 2) != 0);
}

void c2eBrainLobe::write(ostream &s) const {
	for (int i = 0; i < 4; i++) s << id[i];

	write16(s, updatetime);
	write16(s, x);
	write16(s, y);

	s << width << height;
	s << red << green << blue;
	s << WTA << tissue;

	for (int i = 0; i < 8; i++) s << spare[i];
	for (int i = 0; i < 48; i++) s << initialiserule[i];
	for (int i = 0; i < 48; i++) s << updaterule[i];
}

void c2eBrainLobe::read(istream &s) {
	for (int i = 0; i < 4; i++) s >> id[i];

	updatetime = read16(s);
	x = read16(s);
	y = read16(s);

	s >> width >> height;
	s >> red >> green >> blue;
	s >> WTA >> tissue;

	for (int i = 0; i < 8; i++) s >> spare[i];
	for (int i = 0; i < 48; i++) s >> initialiserule[i];
	for (int i = 0; i < 48; i++) s >> updaterule[i];
}

void c2eBrainTract::write(ostream &s) const {
	write16(s, updatetime);
	for (int i = 0; i < 4; i++) s << srclobe[i];
	write16(s, srclobe_lowerbound);
	write16(s, srclobe_upperbound);
	write16(s, src_noconnections);
	for (int i = 0; i < 4; i++) s << destlobe[i];
	write16(s, destlobe_lowerbound);
	write16(s, destlobe_upperbound);
	write16(s, dest_noconnections);
	s << randomdendrites << norandomconnections;
	for (int i = 0; i < 8; i++) s << spare[i];
	for (int i = 0; i < 48; i++) s << initialiserule[i];
	for (int i = 0; i < 48; i++) s << updaterule[i];
}

void c2eBrainTract::read(istream &s) {
	updatetime = read16(s);
	for (int i = 0; i < 4; i++) s >> srclobe[i];
	srclobe_lowerbound = read16(s);
	srclobe_upperbound = read16(s);
	src_noconnections = read16(s);
	for (int i = 0; i < 4; i++) s >> destlobe[i];
	destlobe_lowerbound = read16(s);
	destlobe_upperbound = read16(s);
	dest_noconnections = read16(s);
	s >> randomdendrites >> norandomconnections;
	for (int i = 0; i < 8; i++) s >> spare[i];
	for (int i = 0; i < 48; i++) s >> initialiserule[i];
	for (int i = 0; i < 48; i++) s >> updaterule[i];
}

void creatureAppearance::write(ostream &s) const {
	s << part << variant;
	if (cversion > 1) s << species;
}

void creatureAppearance::read(istream &s) {
	s >> part >> variant;
	if (cversion > 1) s >> species;
}

void creatureFacialExpression::write(ostream &s) const {
	write16(s, expressionno);
	s << weight;

	for (int i = 0; i < 4; i++) {
		s << drives[i] << amounts[i];
	}
}

void creatureFacialExpression::read(istream &s) {
	expressionno = read16(s);
	s >> weight;

	for (int i = 0; i < 4; i++) {
		s >> drives[i] >> amounts[i];
	}
}

void creatureGait::write(ostream &s) const {
	s << drive;

	for (int i = 0; i < gaitLength(); i++) {
		s << pose[i];
	}
}

void creatureGait::read(istream &s) {
	s >> drive;

	for (int i = 0; i < gaitLength(); i++) {
		s >> pose[i];
	}
}

void creatureGenus::write(ostream &s) const {
	s << genus;

	const char *b;

	// TODO: we read past the end of the returned buffer here!
	b = mum.c_str();
	for (int i = 0; i < ((cversion == 3) ? 32 : 4); i++) s << b[i];
	b = dad.c_str();
	for (int i = 0; i < ((cversion == 3) ? 32 : 4); i++) s << b[i];
}

void creatureGenus::read(istream &s) {
	s >> genus;

	char buf[33];
	unsigned int len = ((cversion == 3) ? 32 : 4);

	s.read(buf, len);
	buf[len] = 0;
	mum = (char *)buf;

	s.read(buf, len);
	buf[len] = 0;
	dad = (char *)buf;
}

void creatureInstinct::write(ostream &s) const {
	for (int i = 0; i < 3; i++) {
		s << lobes[i] << neurons[i];
	}

	s << action;
	s << drive;
	s << level;
}

void creatureInstinct::read(istream &s) {
	for (int i = 0; i < 3; i++) {
		s >> lobes[i] >> neurons[i];
	}

	s >> action;
	s >> drive;
	s >> level;
}

void creaturePigment::write(ostream &s) const {
	s << color << amount;
}

void creaturePigment::read(istream &s) {
	s >> color >> amount;
}

void creaturePigmentBleed::write(ostream &s) const {
	s << rotation << swap;
}

void creaturePigmentBleed::read(istream &s) {
	s >> rotation >> swap;
}

void creaturePose::write(ostream &s) const {
	s << poseno;

	for (int i = 0; i < poseLength(); i++) {
		s << pose[i];
	}
}

void creaturePose::read(istream &s) {
	s >> poseno;

	for (int i = 0; i < poseLength(); i++) {
		s >> pose[i];
	}
}

void creatureStimulus::write(ostream &s) const {
	s << stim << significance << sensoryneuron << intensity << features;

	for (int i = 0; i < 4; i++) {
		s << drives[i] << amounts[i];
	}
}

void creatureStimulus::read(istream &s) {
	s >> stim >> significance >> sensoryneuron >> intensity >> features;

	for (int i = 0; i < 4; i++) {
		s >> drives[i] >> amounts[i];
	}
}

void oldBrainLobe::write(ostream &s) const {
	s << x << y << width << height << perceptflag << nominalthreshold << leakagerate << reststate << inputgain;
	s.write(staterule, (cversion == 1) ? 8 : 12);
	s << flags;

	s << dendrite1 << dendrite2;
}

void oldBrainLobe::read(istream &s) {
	s >> x >> y >> width >> height >> perceptflag >> nominalthreshold >> leakagerate >> reststate >> inputgain;
	s.read(staterule, (cversion == 1) ? 8 : 12);
	s >> flags;

	s >> dendrite1 >> dendrite2;
}

void organGene::write(ostream &s) const {
	s << clockrate << repairrate << lifeforce << biotickstart << atpdamagecoefficient;

	// iterate through children
	for (vector<gene *>::iterator x = ((organGene *)this)->genes.begin(); x != ((organGene *)this)->genes.end(); x++)
		s << **x;
}

void organGene::read(istream &s) {
	s >> clockrate >> repairrate >> lifeforce >> biotickstart >> atpdamagecoefficient;
}

ostream &operator << (ostream &s, const oldDendriteInfo &i) {
	s << i.srclobe << i.min << i.max << i.spread << i.fanout << i.minLTW << i.maxLTW;
	s << i.minstr << i.maxstr << i.migrateflag << i.relaxsuscept << i.relaxSTW << i.LTWgainrate;

	s << i.strgain;
	s.write(i.strgainrule, (i.cversion == 1) ? 8 : 12);
	s << i.strloss;
	s.write(i.strlossrule, (i.cversion == 1) ? 8 : 12);
	s.write(i.susceptrule, (i.cversion == 1) ? 8 : 12);
	s.write(i.relaxrule, (i.cversion == 1) ? 8 : 12);

	if (i.cversion == 2) {
		s.write(i.backproprule, 12);
		s.write(i.forproprule, 12);
	}

	return s;
}

istream &operator >> (istream &s, oldDendriteInfo &i) {
	s >> i.srclobe >> i.min >> i.max >> i.spread >> i.fanout >> i.minLTW >> i.maxLTW;
	s >> i.minstr >> i.maxstr >> i.migrateflag >> i.relaxsuscept >> i.relaxSTW >> i.LTWgainrate;

	s >> i.strgain;
	s.read(i.strgainrule, (i.cversion == 1) ? 8 : 12);
	s >> i.strloss;
	s.read(i.strlossrule, (i.cversion == 1) ? 8 : 12);
	s.read(i.susceptrule, (i.cversion == 1) ? 8 : 12);
	s.read(i.relaxrule, (i.cversion == 1) ? 8 : 12);

	if (i.cversion == 2) {
		s.read(i.backproprule, 12);
		s.read(i.forproprule, 12);
	}

	return s;
}
