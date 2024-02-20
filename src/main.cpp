#include <iostream>
#include <fstream>
#include <vector>

#include <SFML/Graphics.hpp>
#include "imgui.h"
#include "imgui-SFML.h"

class Shape : public sf::Drawable
{
public:
    std::string name;
    sf::Vector2f velocity;
    sf::Shape* shape = nullptr;
    sf::Text text;
    bool enabled = true;
    float color[3] = { 0.0f, 0.0f, 0.0f };
    float scale = 1.0f;

    ~Shape()
    {
        if (shape != nullptr)
            delete shape;
    }

    void update()
    {
        if (enabled)
        {
            sf::Vector2f pos = shape->getPosition();
            pos += velocity;
            shape->setPosition(pos);
            shape->setFillColor(sf::Color(color[0] * 255, color[1] * 255, color[2] * 255));
            shape->setScale(scale, scale);
            centerText();
        }
    }

    void checkBounds(sf::Vector2i& resolution)
    {
        sf::FloatRect box = shape->getGlobalBounds();
        sf::Vector2f pos = shape->getPosition();

        if (box.top < 0.0f)
        {
            velocity.y *= -1.0f;
            shape->setPosition(pos.x, 0.0f);
        }
        else if (box.top + box.height > resolution.y)
        {
            velocity.y *= -1.0f;
            shape->setPosition(pos.x, resolution.y - box.height);
        }

        if (box.left < 0.0f)
        {
            velocity.x *= -1.0f;
            shape->setPosition(0.0f, pos.y);
        }
        else if (box.left + box.width > resolution.x)
        {
            velocity.x *= -1.0f;
            shape->setPosition(resolution.x - box.width, pos.y);
        }
    }

    virtual void load(std::ifstream& fin, sf::Font& font) = 0;

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        if (enabled)
        {
            target.draw(*shape);
            target.draw(text);
        }
    }

protected:
    void centerText()
    {
        sf::FloatRect textLocalBounds = text.getLocalBounds();
        sf::FloatRect shapeGlobalBounds = shape->getGlobalBounds();

        float x = shapeGlobalBounds.left + ((shapeGlobalBounds.width - textLocalBounds.width) / 2) - textLocalBounds.left;
        float y = shapeGlobalBounds.top + ((shapeGlobalBounds.height - textLocalBounds.height) / 2) - textLocalBounds.top;

        text.setPosition(x, y);
    }
   
};

class Circle : public Shape
{
public:
    void load(std::ifstream& fin, sf::Font& font) override
    {
        float posX, posY, radius;

        fin >> name >> posX >> posY >> velocity.x >> velocity.y >> color[0] >> color[1] >> color[2] >> radius;
        sf::CircleShape* c = new sf::CircleShape();
        c->setPosition(posX, posY);
        color[0] /= 255;
        color[1] /= 255;
        color[2] /= 255;
        c->setRadius(radius);
        shape = c;
        shape->setFillColor(sf::Color(color[0] * 255, color[1] * 255, color[2] * 255));

        text.setString(name);
        text.setFont(font);
        text.setCharacterSize(18);
        text.setFillColor(sf::Color::White);
        centerText();
    }
};

class Rectangle : public Shape
{
public:
    void load(std::ifstream& fin, sf::Font& font) override
    {
        float posX, posY, width, height;

        fin >> name >> posX >> posY >> velocity.x >> velocity.y >> color[0] >> color[1] >> color[2] >> width >> height;
        sf::RectangleShape* rect = new sf::RectangleShape();
        rect->setPosition(posX, posY);
        color[0] /= 255;
        color[1] /= 255;
        color[2] /= 255;
        rect->setSize(sf::Vector2f(width, height));
        shape = rect;
        shape->setFillColor(sf::Color(color[0] * 255, color[1] * 255, color[2] * 255));

        text.setString(name);
        text.setFont(font);
        text.setCharacterSize(20);
        text.setFillColor(sf::Color::White);
        centerText();
    }
};

static void loadConfig(sf::Vector2i& res, sf::Font& font, std::vector<Shape*>& shapes)
{
    std::ifstream fin("assets/config.txt");
    std::string temp;

    while (fin >> temp)
    {
        std::cout << temp << std::endl;

        if (temp == "Circle")
        {
            Circle* c = new Circle();
            c->load(fin, font);
            shapes.push_back(c);
        }
        else if (temp == "Rectangle")
        {
            Rectangle* r = new Rectangle();
            r->load(fin, font);
            shapes.push_back(r);
        }
        else if (temp == "Window")
        {
            fin >> res.x >> res.y;
        }
        else if (temp == "Font")
        {
            std::string path;
            fin >> path;
            if (!font.loadFromFile(path))
            {
                std::cerr << "Could not load font." << std::endl;
                exit(-1);
            }

        }
    }
}

int main(int argc, char* argv[])
{
    sf::Font font;
    sf::Vector2i resolution(1280, 720);
    std::vector<Shape*> shapes;
    loadConfig(resolution, font, shapes);

    sf::RenderWindow window(sf::VideoMode(resolution.x, resolution.y), "SFML works!");

    ImGui::SFML::Init(window);
    window.setVerticalSyncEnabled(true);
    sf::Clock clock;

    //ImGui::GetStyle().ScaleAllSizes(2.0f);

    char textBuffer[255] = "New name";

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            ImGui::SFML::ProcessEvent(event);
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed)
            {
                std::cout << "Key pressed with code = " << event.key.code << std::endl;

                if (event.key.code == sf::Keyboard::Escape)
                    window.close();
            }
        }

        sf::Time dt = clock.restart();

        ImGui::SFML::Update(window, dt);

        //ImGui::ShowDemoWindow();

        ImGui::Begin("Title");
        ImGui::SetWindowFontScale(2.0f);
        if (ImGui::TreeNode("Shapes"))
        {
            for (auto shape : shapes)
            {
                if (ImGui::TreeNode((void*)shape, shape->name.c_str()))
                {
                    ImGui::Checkbox("Draw shape", &shape->enabled);
                    ImGui::SliderFloat("x velocity", &shape->velocity.x, -8.0f, 8.0f, nullptr);
                    ImGui::SliderFloat("y velocity", &shape->velocity.y, -8.0f, 8.0f);
                    ImGui::SliderFloat("scale", &shape->scale, 0.0f, 4.0f);
                    ImGui::ColorEdit3("Color", shape->color);
                    ImGui::InputText("Name", textBuffer, 255);
                    if (ImGui::Button("Set name"))
                    {
                        shape->name = textBuffer;
                        shape->text.setString(shape->name);
                    }
                    ImGui::TreePop();
                }
            }

            ImGui::TreePop();
        }
        ImGui::End();

        window.clear();

        for (auto shape : shapes)
        {
            shape->update();
            shape->checkBounds(resolution);
            window.draw(*shape);
        }
        
        ImGui::SFML::Render(window);
        window.display();
    }

    for (auto shape : shapes)
        delete shape;

    ImGui::SFML::Shutdown();
    return 0;
}