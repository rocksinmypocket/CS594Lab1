#ifndef JGRAPH_H
#define JGRAPH_H

#include <string>
#include <unistd.h>
#include <ext/stdio_filebuf.h>
#include <vector>
#include <sys/wait.h>
#include <memory>
#include <cmath>

using namespace std;

/*
 * This class is a C++ interface for JGraph. A handful of JGraph functions 
 * are not implemented, and many have not been tested thoroughly, if at all.
 * 
 * In terms of usage, Canvas hold the top level commands, with everything
 * else under it. Review the JGraph documentation to get an idea for what
 * classes do what.
 * 
 * To print the JGraph, call jgraphToJPG with the first argument as a canvas,
 * and the second as a string with the output file name. The third argument
 * is an optional boolean, which can be set to false to disable waiting for
 * JGraph and convert/gs to return.
 */

class JGraph {
private:
	union Pipe {
	public:
		Pipe() {
			if (pipe(array)) {
				throw system_error(errno, generic_category());
			}
		}
		Pipe(int output, int input) {
			this->output = output;
			this->input = input;
		}
		struct {
			int output;
			int input;
		};
	private:
		int array[2];
	};
public:
	class Color {
	public:
		Color() {
			R = NAN;
			G = NAN;
			B = NAN;
		}
		Color(float R, float G, float B) {
			this->R = R;
			this->G = G;
			this->B = B;
		}
		bool empty() {
			return isnan(R) || isnan(G) || isnan(B);
		}
		float R;
		float G;
		float B;
	};
	static Color Gray(float L) {
		return Color(L, L, L);
	}
	template <typename T>
	struct Size {
		T width;
		T height;
	};
	template <typename T>
	struct Point {
		T x;
		T y;
	};
	template <typename T>
	struct ErrorPoint {
		T x;
		T y;
		T low;
		T high;
	};
	template <typename T>
	struct Rectangle {
		float X;
		float Y;
		float width;
		float height;
	};
	class Text {
	public:
		enum class HorizontalJustification {
			Default,
			left,
			center,
			right
		};

		enum class VerticalJustification {
			Default,
			top,
			center,
			bottom
		};

		Text() {
			position.x = NAN;
			position.y = NAN;
			font = "";
			size = NAN;
			line_spacing = NAN;
			rotate_angle = NAN;
			hor_just = HorizontalJustification::Default;
			ver_just = VerticalJustification::Default;
			content = "";
		}

		string content;
		Point<float> position;
		string font;
		float size;
		float line_spacing;
		HorizontalJustification hor_just;
		VerticalJustification ver_just;
		float rotate_angle;
		Color color;

		bool empty() {
			if (!isnan(position.x)) {
				return false;
			}
			if (!isnan(position.y)) {
				return false;
			}
			if (!font.empty()) {
				return false;
			}
			if (!isnan(size)) {
				return false;
			}
			if (!isnan(line_spacing)) {
				return false;
			}
			if (hor_just != HorizontalJustification::Default) {
				return false;
			}
			if (ver_just != VerticalJustification::Default) {
				return false;
			}
			if (!isnan(rotate_angle)) {
				return false;
			}
			if (!color.empty()) {
				return false;
			}
			if (!content.empty()) {
				return false;
			}
			return true;
		}
		void toJGraph(ostream& out) {
			if (!isnan(position.x)) {
				out << "x " << position.x << " ";
			}
			if (!isnan(position.y)) {
				out << "y " << position.y << " ";
			}
			if (!font.empty()) {
				out << "font " << font << " ";
			}
			if (!isnan(size)) {
				out << "fontsize " << size << " ";
			}
			if (!isnan(line_spacing)) {
				out << "linesep " << line_spacing << " ";
			}
			char justif;
			if (hor_just != HorizontalJustification::Default) {
				switch (hor_just) {
				case HorizontalJustification::left:
					justif = 'l';
					break;
				case HorizontalJustification::center:
					justif = 'c';
					break;
				case HorizontalJustification::right:
					justif = 'r';
					break;
				}
				out << "hj" << justif << " ";
			}
			if (ver_just != VerticalJustification::Default) {
				switch (ver_just) {
				case VerticalJustification::top:
					justif = 't';
					break;
				case VerticalJustification::center:
					justif = 'c';
					break;
				case VerticalJustification::bottom:
					justif = 'b';
					break;
				}
				out << "vj" << justif << " ";
			}
			if (!isnan(rotate_angle)) {
				out << "rotate " << rotate_angle << " ";
			}
			if (!color.empty()) {
				out << "lcolor " << color.R << " " << color.G << " " << color.B << " ";
			}
			if (!content.empty()) {
				out << ": ";
				for (int i = 0; i < content.size(); i++) {
					if (content[i] == '\n') {
						out << '\\';
					}
					out << content[i];
				}
			}
		}
	};
	class Axis {
	public:
		enum class Scale {
			Default,
			linear,
			log
		};
		enum class HashLabelFormat {
			Default,
			floating_point,
			shortest_upper,
			shortest_lower,
			scientific_upper,
			scientific_lower
		};

		Axis() {
			scale = Scale::Default;
			min = NAN;
			max = NAN;
			size_inches = NAN;
			log_base = 10;
			hash_spacing = NAN;
			hash_start = NAN;
			minor_hash_count = -2;
			label_precision = -1;
			label_format = HashLabelFormat::Default;
			draw_at = NAN;
			draw = true;
			grid_lines = false;
			minor_grid_lines = false;
			hash_scale = NAN;
			hash_axis_distance = NAN;
			hash_label_distance = NAN;
			auto_hash_marks = true;
			auto_hash_labels = true;
			draw_axis = true;
			draw_hash_marks = true;
			draw_hash_labels = true;
		}

		Scale scale;
		float min;
		float max;
		float size_inches;
		float log_base;
		float hash_spacing;
		float hash_start;
		int minor_hash_count;
		int label_precision;
		HashLabelFormat label_format;
		Text label;
		float draw_at;
		bool draw;
		bool grid_lines;
		bool minor_grid_lines;
		Color color;
		Color grid_color;
		Color mgrid_color;
		vector<float> manual_hashes;
		vector<float> manual_minor_hashes;
		vector<pair<string, float> > hash_labels;
		Text hash_label_format; // consider sanitizing for content, x, and y values
		float hash_scale;
		float hash_axis_distance;
		float hash_label_distance;
		bool auto_hash_marks;
		bool auto_hash_labels;
		bool draw_axis;
		bool draw_hash_marks;
		bool draw_hash_labels;

		bool empty() {
			if (!draw) {
				return false;
			}
			if (scale != Scale::Default) {
				return false;
			}
			if (!isnan(size_inches)) {
				return false;
			}
			if (!isnan(min) || !isnan(max)) {
				return false;
			}
			if (label_format != HashLabelFormat::Default || label_precision >= 0) {
				return false;
			}
			if (!isnan(hash_spacing) || !isnan(hash_start)) {
				return false;
			}
			if (minor_hash_count >= -1) {
				return false;
			}
			if (!label.empty()) {
				return false;
			}
			if (!isnan(draw_at)) {
				return false;
			}
			if (grid_lines) {
				return false;
			}
			if (minor_grid_lines) {
				return false;
			}
			// consider color
			if (!manual_hashes.empty()) {
				return false;
			}
			if (!manual_minor_hashes.empty()) {
				return false;
			}
			if (!hash_label_format.empty()) {
				return false;
			}
			if (!hash_labels.empty()) {
				return false;
			}
			if (!isnan(hash_scale)) {
				return false;
			}
			if (!isnan(hash_axis_distance)) {
				return false;
			}
			if (!isnan(hash_label_distance)) {
				return false;
			}
			if (!auto_hash_marks) {
				return false;
			}
			if (!auto_hash_labels) {
				return false;
			}
			if (!draw_axis) {
				return false;
			}
			if (!draw_hash_marks) {
				return false;
			}
			if (!draw_hash_labels) {
				return false;
			}
			return true;
		}
		void toJGraph(ostream& out) {
			string indent = string(1,'\t');
			if (!draw) {
				out << indent << "nodraw" << endl;
			}
			if (scale == Scale::linear) {
				out << indent << "linear" << endl;
			}
			else if (scale == Scale::log) {
				out << indent << "log log_base" << log_base << endl;
			}
			if (!isnan(size_inches)) {
				out << indent << "size " << size_inches << endl;
			}
			if (!isnan(min) || !isnan(max)) {
				out << indent;
				if (!isnan(min)) {
					out << "min " << min << " ";
				}
				if (!isnan(max)) {
					out << "max " << max << " ";
				}
				out << endl;
			}
			if (label_format != HashLabelFormat::Default || label_precision >= 0) {
				out << indent;
				if (label_format != HashLabelFormat::Default) {
					char print_format;
					switch (label_format) {
					case HashLabelFormat::floating_point:
						print_format = 'f';
						break;
					case HashLabelFormat::shortest_upper:
						print_format = 'G';
						break;
					case HashLabelFormat::shortest_lower:
						print_format = 'g';
						break;
					case HashLabelFormat::scientific_upper:
						print_format = 'E';
						break;
					case HashLabelFormat::scientific_lower:
						print_format = 'e';
						break;
					}
					out << "hash_format " << print_format << " ";
				}
				if (label_precision >= 0) {
					out << "precision " << label_precision << " ";
				}
				out << endl;
			}
			if (!isnan(hash_spacing) || !isnan(hash_start)) {
				out << indent;
				if (!isnan(hash_spacing)) {
					out << "hash " << hash_spacing << " ";
				}
				if (!isnan(hash_start)) {
					out << "shash " << hash_start << " ";
				}
				out << endl;
			}
			if (minor_hash_count >= -1) {
				out << indent << "mhash " << minor_hash_count << endl;
			}
			if (!label.empty()) {
				out << indent << "label ";
				label.toJGraph(out);
				out << endl;
			}
			if (!isnan(draw_at)) {
				out << indent << draw_at << endl;
			}
			if (grid_lines) {
				out << indent << "grid_lines ";
				if (!grid_color.empty()) {
					out << "grid_color " <<  grid_color.R << " " << grid_color.G << " " << grid_color.B << " ";
				}
				out << endl;
			}
			if (minor_grid_lines) {
				out << indent << "mgrid_lines ";
				if (!mgrid_color.empty()) {
					out << "mgrid_color " << mgrid_color.R << " " << mgrid_color.G << " " << mgrid_color.B << " ";
				}
				out << endl;
			}
			if (!color.empty()) {
				out << indent << "color " << color.R << " " << color.G << " " << color.B << endl;
			}
			if (!manual_hashes.empty()) {
				out << indent;
				for (float hash_pos : manual_hashes) {
					out << "hash_at " << hash_pos << " ";
				}
				out << endl;
			}
			if (!manual_minor_hashes.empty()) {
				out << indent;
				for (float hash_pos : manual_minor_hashes) {
					out << "mhash_at " << hash_pos << " ";
				}
				out << endl;
			}
			if (!hash_label_format.empty()) {
				out << indent << "hash_labels ";
				hash_label_format.toJGraph(out);
				out << endl;
			}
			if (!hash_labels.empty()) {
				out << indent;
				for (pair<string, float> hash_label : hash_labels) {
					out << "hash_label : " << hash_label.first << " at " << hash_label.second << " ";
				}
				out << endl;
			}
			if (!isnan(hash_scale)) {
				out << indent << "hash_scale " << hash_scale << endl;
			}
			if (!isnan(hash_axis_distance)) {
				out << indent << "draw_hash_marks_at " << hash_axis_distance << endl;
			}
			if (!isnan(hash_label_distance)) {
				out << indent << "draw_hash_labels_at " << hash_label_distance << endl;
			}
			if (!auto_hash_marks) {
				out << indent << "no_auto_hash_marks" << endl;
			}
			if (!auto_hash_labels) {
				out << indent << "no_auto_hash_labels" << endl;
			}
			if (!draw_axis) {
				out << indent << "no_draw_axis" << endl;
			}
			if (!draw_hash_marks) {
				out << indent << "no_draw_hash_axis" << endl;
			}
			if (!draw_hash_labels) {
				out << indent << "no_draw_hash_labels" << endl;
			}
		}
	};
	class Mark {
	public:
		Size<float> size;
		float rotate_angle;

		Mark() {
			size.width = NAN;
			size.height = NAN;
			rotate_angle = NAN;
		}

		virtual Mark* Clone() = 0;

		virtual void toJGraph(ostream& out) = 0;
	};
	class ShapeMark : public Mark {
	private:
		inline string shapeTypeToString() {
			switch (type) {
			case Type::circle: return "circle";
			case Type::box: return "box";
			case Type::diamond: return "diamond";
			case Type::triangle: return "triangle";
			case Type::x: return "x";
			case Type::cross: return "cross";
			case Type::ellipse: return "ellipse";
			case Type::xbar: return "xbar";
			case Type::ybar: return "ybar";
			default: return "";
			}
		}

		inline string fillPatternToString() {
			switch (pattern) {
			case FillPattern::solid: return "solid";
			case FillPattern::estripe: return "estripe";
			case FillPattern::stripe: return "stripe";
			default: return "";
			}
		}
	public:
		enum class FillPattern {
			Default,
			solid,
			stripe,
			estripe
		};

		enum class Type {
			Default,
			circle, // fillable
			box, // fillable
			diamond, // fillable
			triangle, // fillable
			x, // probably not fillable
			cross, // probably not fillable
			ellipse, // fillable
			xbar, // fillable
			ybar, // fillable
			none
		};

		Type type;
		Color color;
		FillPattern pattern;
		float fill_rotate_angle;
		//float mark_rotate_angle;

		ShapeMark() {
			type = Type::none;
			pattern = FillPattern::Default;
			fill_rotate_angle = 0;
			//mark_rotate_angle = NAN;
		}

		ShapeMark* Clone()
		{
			return new ShapeMark(*this);
		}

		virtual void toJGraph(ostream& out) {
			if (type == Type::none) {
				out << "marktype none ";
				return;
			}

			if (type != Type::Default) {
				string markType = shapeTypeToString();
				out << "marktype " << markType << " ";
			}
			// Mark size
			if (!(isnan(size.width) || isnan(size.height))) out << "marksize " << size.width << " " << size.height << " ";

			// Mark Rotation
			if (!(isnan(rotate_angle))) out << "mrotate " << rotate_angle << " ";

			// Mark shape color fill
			if (!(color.empty())) out << "cfill " << color.R << " " << color.G << " " << color.B << " ";

			// Mark Patterns
			if (pattern != FillPattern::Default) {
				out << "pattern " << fillPatternToString() << " " << fill_rotate_angle << " ";
			}

		}
	};
	class TextMark : public Mark {
	public:
		Text text;

		TextMark* Clone()
		{
			return new TextMark(*this);
		}

		virtual void toJGraph(ostream& out) {
			out << "marktype text ";
			text.toJGraph(out);
			out << endl;
		}
	};
	class PostscriptRawMark : public Mark {
	public:
		string script;

		PostscriptRawMark* Clone()
		{
			return new PostscriptRawMark(*this);
		}

		virtual void toJGraph(ostream& out) {
			out << "postscript : ";
			out << script << " ";

			// Mark size
			if (!(isnan(size.width) || isnan(size.height))) out << "marksize " << size.width << " " << size.height;

			// Mark Rotation
			if (!(isnan(rotate_angle))) out << "mrotate " << rotate_angle << " ";
		}
	};
	class PostscriptFileMark : public Mark {
	public:
		string fileName;
		bool encapsulated;

		PostscriptFileMark() {
			fileName = "";
			encapsulated = false;
		}

		PostscriptFileMark(string file, bool eps) {
			fileName = file;
			encapsulated = eps;
		}

		PostscriptFileMark* Clone()
		{
			return new PostscriptFileMark(*this);
		}

		virtual void toJGraph(ostream& out) {
			// postscript for encapsulated/files
			out << ((encapsulated) ? "eps " : "postscript ") << fileName << " ";

			// Mark size
			if (!(isnan(size.width) || isnan(size.height))) out << "marksize " << size.width << " " << size.height;

			// Mark Rotation
			if (!(isnan(rotate_angle))) out << "mrotate " << rotate_angle << " ";
		}
	};
	class GeneralMark : public Mark {
	private:
		inline string fillPatternToString() {
			switch (pattern) {
			case FillPattern::solid: return "solid";
			case FillPattern::estripe: return "estripe";
			case FillPattern::stripe: return "stripe";
			default: return "";
			}
		}

		inline string typeToString() {
			switch (type) {
			case Type::general: return "general";
			case Type::general_nf: return "general_nf";
			case Type::general_bez: return "general_bez";
			case Type::general_bez_nf: return "general_bez_nf";
			default: return "";
			}

		}
	public:
		enum class FillPattern {
			Default,
			solid,
			stripe,
			estripe
		};

		enum class Type {
			general,
			general_nf,
			general_bez,
			general_bez_nf
		};

		Color color;
		FillPattern pattern;
		Type type;
		float fill_rotate_angle;

		GeneralMark() {
			pattern = FillPattern::Default;
			type = Type::general;
			fill_rotate_angle = 0;
		}

		GeneralMark* Clone()
		{
			return new GeneralMark(*this);
		}

		vector<Point<float>> points; // probably implement some checking on this, either in the constructor or in toJGraph; must be 3n+1 for bezier curves

		virtual void toJGraph(ostream& out) {
			// Check if bezier points number is valid
			if ((points.size() % 3) != 1 && (type == Type::general_bez || type == Type::general_bez_nf)) return;

			// Print points
			out << "gmarks ";
			for (int i = 0; i < points.size(); i++) {
				out << points[i].x << " " << points[i].y << " ";
			}

			// Marktype
			out << "marktype " << typeToString() << " ";

			// If fill, specify fill information
			if (type == Type::general || type == Type::general_bez) {
				// Mark shape color fill
				if (!(color.empty())) out << "cfill " << color.R << " " << color.G << " " << color.B << " ";

				// Mark Patterns
				if (pattern != FillPattern::Default) {
					out << "pattern " << fillPatternToString() << " " << fill_rotate_angle << " ";
				}
			}

			// Mark size
			if (!(isnan(size.width) || isnan(size.height))) out << "marksize " << size.width << " " << size.height << " ";

			// Mark Rotation
			if (!(isnan(rotate_angle))) out << "mrotate " << rotate_angle << " ";
		}
	};
	class Arrows {
	private:
		inline string fillPatternToString() {
			switch (apattern) {
			case FillPattern::solid: return "solid";
			case FillPattern::estripe: return "estripe";
			case FillPattern::stripe: return "stripe";
			default: return "";
			}
		}

		inline string larrowTypetoString() {
			switch (larrow) {
			case ArrowType::noarrow: return "nolarrow";
			case ArrowType::noarrows: return "nolarrows";
			case ArrowType::arrows: return "larrows";
			case ArrowType::arrow: return "larrow";
			default: return "";
			}
		}

		inline string rarrowTypetoString() {
			switch (rarrow) {
			case ArrowType::noarrow: return "norarrow";
			case ArrowType::noarrows: return "norarrows";
			case ArrowType::arrows: return "rarrows";
			case ArrowType::arrow: return "rarrow";
			default: return "";
			}
		}
	public:
		enum class FillPattern {
			solid,
			stripe,
			estripe,
			Default
		};

		enum class ArrowType {
			noarrow,
			noarrows,
			arrow,
			arrows,
			Default
		};

		FillPattern apattern;
		Color acfill;
		Size<float> asize;
		ArrowType larrow;
		ArrowType rarrow;
		float fill_rotate_angle;

		Arrows() {
			apattern = FillPattern::Default;
			asize.height = NAN;
			asize.width = NAN;
			larrow = ArrowType::Default;
			rarrow = ArrowType::Default;
			fill_rotate_angle = 0;
		}

		void toJGraph(ostream& out) {
			if (larrow != ArrowType::Default) {
				out << larrowTypetoString() << " ";
			}
			if (rarrow != ArrowType::Default) {
				out << rarrowTypetoString() << " ";
			}

			// Only add fill/pattern if larrow is not noarrow(s) and rarrow is not noarrow(s)
			if ((larrow != ArrowType::noarrow && larrow != ArrowType::noarrows) ||
				(rarrow != ArrowType::noarrow && rarrow != ArrowType::noarrows))
			{
				// Mark shape color fill
				if (!(acfill.empty())) out << "acfill " << acfill.R << " " << acfill.G << " " << acfill.B << " ";

				// Arrow size
				if (!(isnan(asize.width) || isnan(asize.height))) out << "asize " << asize.width << " " << asize.height;

				// Arrow Patterns
				if (apattern != FillPattern::Default) {
					out << "apattern " << fillPatternToString() << " " << fill_rotate_angle << " ";
				}

			}
		}
	};
	class Curve {
	private:
		inline string fillPatternToString() {
			switch (polyFill) {
			case FillPattern::solid: return "solid";
			case FillPattern::estripe: return "estripe";
			case FillPattern::stripe: return "stripe";
			default: return "";
			}
		}

		inline string lineTypeToString() {
			switch (lineType) {
			case LineType::dashed: return "dashed";
			case LineType::dotdash: return "dotdash";
			case LineType::dotdotdash: return "dotdotdash";
			case LineType::dotdotdashdash: return "dotdotdashdash";
			case LineType::dotted: return "dotted";
			case LineType::general: return "general";
			case LineType::longdash: return "longdash";
			case LineType::none: return "none";
			case LineType::linesolid: return "solid";
			default: return "";
			}
		}
	public:
		enum class LineType {
			linesolid,
			dotted,
			dashed,
			longdash,
			dotdash,
			dotdotdash,
			dotdotdashdash,
			general,
			none
		};

		enum FillPattern {
			Default,
			solid,
			stripe,
			estripe
		};

		enum class CurveType {
			poly,
			bezier,
			none
		};

		vector<Point<float> > points;
		vector<ErrorPoint<float> > x_error_points;
		vector<ErrorPoint<float> > y_error_points;

		CurveType curve;
		Color polyFillColor;
		FillPattern polyFill;
		float poly_rotate_angle;

		vector<Point<float>> glines;

		Color curveColor;
		LineType lineType;
		float lineThickness;
		unique_ptr<Mark> marks;
		bool clip;
		Text label;
		Arrows arrows;

		Curve() {
			curve = CurveType::none;
			lineType = LineType::linesolid;
			lineThickness = NAN;
			clip = false;
			poly_rotate_angle = 0;
			polyFill = FillPattern::Default;
		}
		Curve(const Curve& other) : marks((other.marks) ? (other.marks->Clone()) : (NULL)) {
			points = other.points;
			x_error_points = other.x_error_points;
			y_error_points = other.y_error_points;
			curve = other.curve;
			polyFillColor = other.polyFillColor;
			polyFill = other.polyFill;
			poly_rotate_angle = other.poly_rotate_angle;
			glines = other.glines;
			curveColor = other.curveColor;
			lineType = other.lineType;
			lineThickness = other.lineThickness;
			clip = other.clip;
			label = other.label;
			arrows = other.arrows;
		}
		void toJGraph(ostream& out) {
			if (curve == CurveType::bezier && (points.size() % 3) != 1) return;

			out << "newcurve ";

			// Curve points
			out << "pts ";
			for (int i = 0; i < points.size(); i++) {
				out << points[i].x << " " << points[i].y << " ";
			}

			// Error Points
			if (x_error_points.size() != 0) {
				out << "x_epts ";
				for (int i = 0; i < x_error_points.size(); i++) {
					out << x_error_points[i].x << " " << x_error_points[i].y << " "
						<< x_error_points[i].low << " "
						<< x_error_points[i].high << " ";
				}
			}
			if (y_error_points.size() != 0) {
				out << "y_epts ";
				for (int i = 0; i < y_error_points.size(); i++) {
					out << y_error_points[i].x << " " << y_error_points[i].y << " "
						<< y_error_points[i].low << " "
						<< y_error_points[i].high << " ";
				}
			}


			// Marks
			marks->toJGraph(out);

			// Linetype
			out << "linetype " << lineTypeToString() << " ";
			if (lineType == LineType::general) {
				for (int i = 0; i < glines.size(); i++) {
					out << glines[i].x << " " << glines[i].y << " ";
				}
			}
			if (!(isnan(lineThickness))) out << "linethickness " << lineThickness << endl;

			// Arrows
			arrows.toJGraph(out);


			// If poly, it has extra parameters
			if (curve == CurveType::poly) {
				out << "poly ";

				// Poly color
				if (!(polyFillColor.empty())) out << "pcfill " << polyFillColor.R << " " << polyFillColor.G << " " << polyFillColor.B << " ";

				// Poly pattern
				if (polyFill != FillPattern::Default) {
					out << "pattern " << fillPatternToString() << " " << poly_rotate_angle << " ";
				}
			}
			else if (curve == CurveType::bezier) {
				out << "bezier ";
			}

			// Color
			if (!(curveColor.empty())) out << "color " << curveColor.R << " " << curveColor.G << " " << curveColor.B << " ";

			// Clip
			if (clip) out << "clip ";
			else out << "noclip ";

			// Label
			label.toJGraph(out);
			out << endl;
		}

	};
	class Legend {
	public:
		Legend() {
			enabled = true;
			line_length = NAN;
			entry_spacing = NAN;
			midspace = NAN;
			custom_entries = false;
		}

		bool enabled;
		float line_length; // in units of x or inches if x is logarithmic
		float entry_spacing; // in units of y or inches if y is logarithmic
		float midspace;
		bool custom_entries;
		Text legend_settings; // maybe add get/set to remove "content" from settings and zero-out rotate, also check for center hor justif

		void toJGraph(ostream& out) {
			out << "legend ";
			if (!enabled) {
				out << "off ";
			}
			else {
				if (!isnan(line_length)) {
					out << "linelength " << line_length << " ";
				}
				if (!isnan(entry_spacing)) {
					out << "linebreak " << entry_spacing << " ";
				}
				if (!isnan(midspace)) {
					out << "midspace " << midspace << " ";
				}
				if (custom_entries) {
					out << "custom ";
				}
				else {
					out << "defaults ";
					legend_settings.toJGraph(out);
				}
			}
		}
	};

	class Graph {
	public:
		Graph() {
			border = false;
			clip = false;
		}
		Axis xaxis;
		Axis yaxis;
		vector<Curve> curves;
		vector<Text> strings;
		Legend legend;
		Text title;
		bool border;
		bool clip;
		//float x_translate;
		//float y_translate;
		//float X;
		//float Y;

		void toJGraph(ostream& out) {
			if (!title.empty()) {
				out << "title ";
				title.toJGraph(out);
				out << endl;
			}
			out << "xaxis\n";
			xaxis.toJGraph(out);
			out << "yaxis\n";
			yaxis.toJGraph(out);
			for (int i = 0; i < curves.size(); i++) {
				curves[i].toJGraph(out);
			}
			for (int i = 0; i < strings.size(); i++) {
				strings[i].toJGraph(out);
				out << endl;
			}
			legend.toJGraph(out);
			if (border) {
				out << "border\n";
			}
			if (clip) {
				out << "noclip\n";
			}
		}
	};
	class Canvas {
	public:
		Canvas() {
			size.width = NAN;
			size.height = NAN;
			bounding_box.X = NAN;
			bounding_box.Y = NAN;
			bounding_box.width = NAN;
			bounding_box.height = NAN;
		}

		vector<Graph> graphs;
		Size<float> size;
		Rectangle<float> bounding_box;
		string preamble;
		string epilogue;

		void toJGraph(ostream& out) {
			if (!preamble.empty()) {
				out << "preamble " << preamble << endl;
			}
			if (!epilogue.empty()) {
				out << "epilogue " << epilogue << endl;
			}
			if (!isnan(size.width) || !isnan(size.height)) {
				if (!isnan(size.width)) {
					out << "X " << size.width << " ";
				}
				if (!isnan(size.height)) {
					out << "Y " << size.height << " ";
				}
				out << endl;
			}
			if (!isnan(bounding_box.X) && !isnan(bounding_box.Y) && !isnan(bounding_box.width) && !isnan(bounding_box.height)) {
				out << "bbox " << bounding_box.X << " " << bounding_box.Y << " "
					<< bounding_box.X + bounding_box.width << " " << bounding_box.Y + bounding_box.height
					<< endl;
			}
			for (int i = 0; i < graphs.size(); i++) {
				out << "newgraph\n";
				graphs[i].toJGraph(out);
			}
			
		}
	};
public:
	static int jgraphToJPG(JGraph::Canvas& canvas, string filename, bool safe=true) {
		Pipe jgraph_in_pipe;

		Pipe jgraph_out_pipe;

		int jg_pid = fork();
		if (!jg_pid) {
			if (!safe) {
				if (fork()) {
					exit(EXIT_SUCCESS);
				}
			}
			close(jgraph_in_pipe.input);
			close(jgraph_out_pipe.output);
			dup2(jgraph_in_pipe.output, STDIN_FILENO);
			dup2(jgraph_out_pipe.input, STDOUT_FILENO);
			vector<const char*> args = { "jgraph", NULL };
			if (execvp(args[0], (char* const*)&args[0])) {
				throw system_error(errno, generic_category());
			}
		}

		close(jgraph_in_pipe.output);
		close(jgraph_out_pipe.input);
		Pipe image_out_pipe;

		int gs_pid = fork();
		if (!gs_pid) {
			if (!safe) {
				if (fork()) {
					exit(EXIT_SUCCESS);
				}
			}
			close(jgraph_in_pipe.input);
			close(image_out_pipe.output);
			dup2(jgraph_out_pipe.output, STDIN_FILENO);
			dup2(image_out_pipe.input, STDOUT_FILENO);
			// commented-out section below is for using ghost-script, which we cannot assume is installed
			//string file_arg = "-sOutputFile=" + filename;
			//vector<const char*> args = { "gs", "-q", "-sDEVICE=jpeg", "-r300","-dEPSCrop","-dBATCH","-dNOPAUSE", file_arg.c_str(), "-", NULL };
			string file_arg = filename;
			vector<const char*> args = { "convert", "-density", "300","-","-quality","100",file_arg.c_str(), NULL };
			if (execvp(args[0], (char* const*)&args[0])) {
				throw system_error(errno, generic_category());
			}
		}

		close(jgraph_out_pipe.output);
		close(image_out_pipe.input);

		close(image_out_pipe.output);

		__gnu_cxx::stdio_filebuf<char> jgraph_tobuf(jgraph_in_pipe.input, ios::out);
		ostream jgout(&jgraph_tobuf);

		canvas.toJGraph(jgout);

		jgraph_tobuf.close();

		int status;
		waitpid(jg_pid, &status, 0);
		waitpid(gs_pid, &status, 0);
		return status;
	}
};

#endif