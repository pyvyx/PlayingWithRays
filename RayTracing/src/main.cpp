#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "SFML/Graphics.hpp"

#include "Arial.h"

struct Line
{
private:
	static const sf::Color s_LightColor;
	static const sf::Color s_ShadowColor;
public:
	sf::Vector2f m_Origin, m_Intersection;
	bool m_Light = false;

	Line() = default;
	Line(const sf::Vector2f& origin) : m_Origin(origin) {}

	void Draw(sf::RenderWindow& window)
	{
		if (m_Light)
		{
			const sf::Vertex line[2] =
			{
				sf::Vertex(m_Origin, s_LightColor),
				sf::Vertex(m_Intersection, s_LightColor)
			};
			window.draw(line, 2, sf::Lines);
		}
		else
		{
			const sf::Vertex line[2] =
			{
				sf::Vertex(m_Origin, s_ShadowColor),
				sf::Vertex(m_Intersection, s_ShadowColor)
			};
			window.draw(line, 2, sf::Lines);
		}
	}
};
const sf::Color Line::s_LightColor = sf::Color(255, 255, 102);
const sf::Color Line::s_ShadowColor = sf::Color(70, 70, 70);


struct Text : public sf::Text
{
private:
	size_t m_Length;
	float m_WindowXSize;
	float m_YPos;
	std::string m_Text;
private:
	void InitObject(const sf::Font& font, float y, const std::string& text)
	{
		setFont(font);
		setCharacterSize(20);
		setFillColor(sf::Color::White);
		setString(m_Text);
		setPosition(m_YPos);
	}

	void UpdatePosition()
	{
		size_t textLength = getString().getSize();
		if (textLength != m_Length)
		{
			setPosition(m_YPos);
			m_Length = textLength;
		}
	}
public:
	Text() : m_Length(0), m_WindowXSize(0.f), m_YPos(0.f) {}
	Text(const sf::Font& font, unsigned int windowXSize, float y, const std::string& text)
		: m_Length(text.size()), m_WindowXSize(static_cast<float>(windowXSize)), m_YPos(y), m_Text(text) 
		{ InitObject(font, y, text); }

	void Init(const sf::Font& font, unsigned int windowXSize, float y, const std::string& text)
	{
		m_Length = text.size();
		m_WindowXSize = static_cast<float>(windowXSize);
		m_YPos = y;
		m_Text = text;
		InitObject(font, y, text); 
	}

	void setPosition(float y)
	{
		sf::FloatRect rect = getLocalBounds();
		sf::Text::setPosition(m_WindowXSize - rect.width - 20.f, y);
	}

	void SetWindowXSize(unsigned int windowXSize) { m_WindowXSize = static_cast<float>(windowXSize); m_Length += 1; UpdatePosition(); }
	
	void Update(size_t validRays) { Update(std::to_string(validRays)); }
	void Update(const std::string& str)
	{
		setString(m_Text + str);
		UpdatePosition();
	}
};


template <size_t S>
struct TextFactory
{
private:
	using cstringr = const std::string&;
	using pair = const std::pair<std::int32_t, size_t>&;
	using ppair = const std::tuple<std::int32_t, bool, cstringr, cstringr>&;
private:
	sf::Font m_Font;
	unsigned int m_WindowSizeX;
	float m_YPos;
	float m_YOffset;
	size_t m_GeneratedTexts;
	std::array<Text, S> m_Texts;
public:
	TextFactory(unsigned int windowXSize, float y, float yOffset)
		: m_WindowSizeX(windowXSize), m_YPos(y), m_YOffset(yOffset), m_GeneratedTexts(0)
	{
		m_Font.loadFromMemory(sg_RawArialData, sg_RawArialDataRelativeSize);
	}

	void DrawTexts(sf::RenderWindow& window) const
	{
		for (size_t i = 0; i < m_GeneratedTexts; ++i)
			window.draw(m_Texts[i]);
	}

	void UpdateWindowSizeX(unsigned int windowXSize)
	{
		m_WindowSizeX = windowXSize;
		for (size_t i = 0; i < m_GeneratedTexts; ++i)
			m_Texts[i].SetWindowXSize(windowXSize);
	}

	void SetFillColor(const sf::Color& color)
	{
		for (size_t i = 0; i < m_GeneratedTexts; ++i)
			m_Texts[i].setFillColor(color);
	}

	void UpdateText(size_t index, const std::string& str)
	{
		m_Texts[index].Update(str);
	}

	void UpdateText(size_t index, size_t value)
	{
		m_Texts[index].Update(value);
	}

	void UpdateText(pair content)
	{
		m_Texts[content.first].Update(content.second);
	}

	void UpdateText(ppair content)
	{
		if (std::get<1>(content))
			UpdateText(std::get<0>(content), std::get<2>(content));
		else
			UpdateText(std::get<0>(content), std::get<3>(content));
	}

	std::int32_t GenerateText(const std::string& text)
	{
		if (m_GeneratedTexts == S)
			return -1;
		m_Texts[m_GeneratedTexts].Init(m_Font, m_WindowSizeX, m_YPos, text);
		++m_GeneratedTexts;
		m_YPos += m_YOffset;
		return m_GeneratedTexts - 1;
	}

	void UpdateTexts(pair rays, pair lightRays, pair shadowRays, pair radius,
		std::pair<std::int32_t, const std::string&> fpsLimit, ppair lightText, ppair shadowText, ppair textColorText)
	{
		UpdateText(rays);
		UpdateText(lightRays); // light ray
		UpdateText(shadowRays); // shadow ray

		UpdateText(radius);
		UpdateText(fpsLimit.first, fpsLimit.second);

		UpdateText(lightText);
		UpdateText(shadowText);
		UpdateText(textColorText);
	}
};


struct DisplayedTexts
{
public:
	const std::string onStr = "On";
	const std::string offStr = "Off";
	const std::string whiteStr = "White";
	const std::string blackStr = "Black";

	TextFactory<9> factory;
	std::int32_t fps, rays, lightRays, shadowRays, radius, light, shadow, textColor, fpsLimit;
	bool lightOn, shadowOn, whiteTextColor;
	unsigned int fpsLimitValue;
	std::string fpsLimitStr;
	size_t lightRaysAmount, shadowRaysAmount;
	size_t radiusValue;

	DisplayedTexts(unsigned int windowSizeX, float yPos, float yOffset)
		: factory(windowSizeX, yPos, yOffset),
		fps(factory.GenerateText("FPS: ")),
		rays(factory.GenerateText("Rays: ")),
		lightRays(factory.GenerateText("Light rays: ")),
		shadowRays(factory.GenerateText("Shadow rays: ")),
		radius(factory.GenerateText("Radius(Up,Down,->,<-): ")),
		light(factory.GenerateText("Light(a): ")),
		shadow(factory.GenerateText("Shadow(d): ")),
		textColor(factory.GenerateText("Text color(e): ")),
		fpsLimit(factory.GenerateText("FPS limit(w/s/f): ")),
		lightOn(true),
		shadowOn(true),
		whiteTextColor(true),
		fpsLimitValue(60),
		fpsLimitStr(std::to_string(fpsLimitValue)),
		lightRaysAmount(0),
		shadowRaysAmount(0),
		radiusValue(0) {}

	void UpdateTexts()
	{
		factory.UpdateTexts(
			{ rays, lightRaysAmount + shadowRaysAmount },
			{ lightRays, lightRaysAmount },
			{ shadowRays, shadowRaysAmount },
			{ radius, radiusValue },
			{ fpsLimit, fpsLimitStr },
			std::make_tuple(light, lightOn, onStr, offStr),
			std::make_tuple(shadow, shadowOn, onStr, offStr),
			std::make_tuple(textColor, whiteTextColor, whiteStr, blackStr)
		);
		lightRaysAmount = 0;
		shadowRaysAmount = 0;
	}

	void UpdateText(int32_t index, size_t value)
	{
		factory.UpdateText(index, value);
	}

	void SetFillColor(const sf::Color& color)
	{
		factory.SetFillColor(color);
	}

	void UpdateWindowSizeX(unsigned int windowXSize)
	{
		factory.UpdateWindowSizeX(windowXSize);
	}

	void DrawTexts(sf::RenderWindow& window)
	{
		factory.DrawTexts(window);
	}
};


void HandleInput(sf::RenderWindow& window, DisplayedTexts& texts, sf::CircleShape& circle)
{
	sf::Event event;
	while (window.pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
			window.close();
		else if (event.type == sf::Event::Resized)
		{
			sf::FloatRect visibleArea(0, 0, static_cast<float>(event.size.width), static_cast<float>(event.size.height));
			window.setView(sf::View(visibleArea));
			texts.UpdateWindowSizeX(window.getSize().x);
		}
		else if (event.type == sf::Event::KeyPressed)
		{
			if (event.key.code == sf::Keyboard::Left)
			{
				float radius = circle.getRadius();
				if (radius != 1.f)
				{
					radius -= 1.f;
					circle.setRadius(radius);
					texts.radiusValue = static_cast<size_t>(radius);
				}
			}
			else if (event.key.code == sf::Keyboard::Right)
			{
				float radius = circle.getRadius() + 1.f;
				circle.setRadius(radius);
				texts.radiusValue = static_cast<size_t>(radius);
			}
			else if (event.key.code == sf::Keyboard::A)
				texts.lightOn = !texts.lightOn;
			else if (event.key.code == sf::Keyboard::D)
				texts.shadowOn = !texts.shadowOn;
			else if (event.key.code == sf::Keyboard::S)
			{
				texts.fpsLimitValue = texts.fpsLimitValue == 1 ? 1 : texts.fpsLimitValue - 1;
				window.setFramerateLimit(texts.fpsLimitValue);
				texts.fpsLimitStr = std::to_string(texts.fpsLimitValue);
			}
			else if (event.key.code == sf::Keyboard::W)
			{
				texts.fpsLimitValue = texts.fpsLimitValue + 1;
				window.setFramerateLimit(texts.fpsLimitValue);
				texts.fpsLimitStr = std::to_string(texts.fpsLimitValue);
			}
			else if (event.key.code == sf::Keyboard::F)
			{
				if (texts.fpsLimitStr == "Off")
				{
					window.setFramerateLimit(texts.fpsLimitValue);
					texts.fpsLimitStr = std::to_string(texts.fpsLimitValue);
				}
				else
				{
					window.setFramerateLimit(UINT_MAX);
					texts.fpsLimitStr = "Off";
				}
			}
			else if (event.key.code == sf::Keyboard::E)
			{
				texts.whiteTextColor = !texts.whiteTextColor;
				if (texts.whiteTextColor)
					texts.SetFillColor(sf::Color::White);
				else
					texts.SetFillColor(sf::Color::Black);
			}
		}
	}

	if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && window.hasFocus())
	{
		const sf::Vector2i mousePos = sf::Mouse::getPosition(window);
		circle.setPosition({ static_cast<float>(mousePos.x) - circle.getRadius(), static_cast<float>(mousePos.y) - circle.getRadius() });
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && window.hasFocus())
	{
		float radius = circle.getRadius() + 1.f;
		circle.setRadius(radius);
		texts.radiusValue = static_cast<size_t>(radius);
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && window.hasFocus())
	{
		float radius = circle.getRadius();
		if (radius != 1.f)
		{
			radius -= 1.f;
			circle.setRadius(radius);
			texts.radiusValue = static_cast<size_t>(radius);
		}
	}
}


void SetProperValues(Line& ray, const sf::Vector2f& origin, const sf::Vector2f& direction, float t1, float t2, bool shadow)
{
	const sf::Vector2f point1 = origin + direction * t1;
	const sf::Vector2f point2 = origin + direction * t2;

	const sf::Vector2f vec1 = point1 - origin;
	const sf::Vector2f vec2 = point2 - origin;

	const float length1 = vec1.x * vec1.x + vec1.y * vec1.y;
	const float length2 = vec2.x * vec2.x + vec2.y * vec2.y;

	if (length1 < length2)
	{
		if (shadow)
		{
			ray.m_Origin = point2;
			ray.m_Intersection = ray.m_Origin + direction * (t2 * 10000.f);
		}
		else
			ray.m_Intersection = point1;
	}
	else
	{
		if (shadow)
		{
			ray.m_Origin = point1;
			ray.m_Intersection = ray.m_Origin + direction * (t1 * 10000.f);
		}
		else
			ray.m_Intersection = point2;
	}
}


std::pair<Line, Line> CalculateRays(const sf::Vector2f& origin, const sf::Vector2f& direction, float radius, const sf::Vector2f& circlePos)
{
	Line light(origin), shadow;
	
	const float a = direction.x * direction.x + direction.y * direction.y;
	const float b = 2.f * origin.x * direction.x - 2.f * direction.x * circlePos.x + 2.f * origin.y * direction.y - 2.f * direction.y * circlePos.y;
	const float c = origin.x * origin.x - 2.f * origin.x * circlePos.x + circlePos.x * circlePos.x + origin.y * origin.y - 2.f * origin.y * circlePos.y + circlePos.y * circlePos.y - radius * radius;

	const float discriminant = b * b - 4.f * a * c;
	if (discriminant < 0)
		return { light, shadow };
	else if (discriminant == 0)
	{
		const float t = -b / (2.f * a);
		light.m_Intersection = origin + direction * t;
		shadow.m_Origin = light.m_Intersection;
		shadow.m_Intersection = shadow.m_Origin + direction * (t * 10000.f); // arbitrary scalar
	}
	else
	{
		const float t1 = (-b + sqrt(discriminant)) / (2.f * a);
		const float t2 = (-b - sqrt(discriminant)) / (2.f * a);
		SetProperValues(light, origin, direction, t1, t2, false);
		SetProperValues(shadow, origin, direction, t1, t2, true);
	}

	light.m_Light = true;
	return { light, shadow };
}


int main()
{
	sf::RenderWindow window(sf::VideoMode(1000, 750), "Rays");

	DisplayedTexts texts(window.getSize().x, 0.f, 30.f);
	window.setFramerateLimit(texts.fpsLimitValue);

	sf::CircleShape circle(100.f, 70);
	circle.setPosition((static_cast<float>(window.getSize().x) / 2.f) - circle.getRadius(), (static_cast<float>(window.getSize().y) / 2.f) - circle.getRadius());
	circle.setFillColor(sf::Color::White);
	texts.radiusValue = static_cast<size_t>(circle.getRadius());

	const float YDir = 0.f;
	const float XDir = 1000.f;
	const float XOff = 0.225f;
	const float YOff = 0.225f;

	float YOffset = YOff;
	float XOffset = XOff;
	sf::Vector2f lightDirection(XDir, YDir);
	const sf::Vector2f lightOrigin(0.f,0.f);

	float fps = 0.f;
	const sf::Clock clock;
	sf::Time previousTime = clock.getElapsedTime();
	sf::Time currentTime;

	while (window.isOpen())
	{
		HandleInput(window, texts, circle);

		window.clear(sf::Color(105, 105, 105, 255));
		window.draw(circle);

		if (texts.lightOn || texts.shadowOn)
		{
			const sf::Vector2f circleRealPosition = circle.getPosition();
			const sf::Vector2f circlePosition(circleRealPosition.x + static_cast<float>(texts.radiusValue), circleRealPosition.y + static_cast<float>(texts.radiusValue));
			
			for (size_t i = 0; i < 5000; ++i)
			{
				if (lightDirection.x < 0.f && lightDirection.y > 750.f)
					break;

				std::pair<Line, Line> lines = CalculateRays(lightOrigin, lightDirection, static_cast<float>(texts.radiusValue), circlePosition);
				if (lines.first.m_Light) // is only true if the ray hits the circle
				{
					if (texts.lightOn)
					{
						lines.first.Draw(window);
						++texts.lightRaysAmount;
					}
					if (texts.shadowOn)
					{
						lines.second.Draw(window);
						++texts.shadowRaysAmount;
					}
				}
				lightDirection.y += YOffset;
				lightDirection.x -= XOffset;
			}

			lightDirection.x = XDir;
			lightDirection.y = YDir;
			YOffset = YOff;
			XOffset = XOff;
		}
		texts.UpdateTexts();

		texts.DrawTexts(window);
		window.display();

		currentTime = clock.getElapsedTime();
		fps = 1.f / (currentTime.asSeconds() - previousTime.asSeconds());
		previousTime = currentTime;
		texts.UpdateText(texts.fps, static_cast<size_t>(fps));
	}

	return 0;
}