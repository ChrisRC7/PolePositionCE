package server;

public interface AbstractFactory {
    Turbo1 createTurbo();
    Vida createVida();
    Hueco createHueco();
    Bala createBala();
}
