#include <gtk/gtk.h>

// 위젯 포인터를 담기 위한 구조체
typedef struct {
    GtkWidget *entry1;
    GtkWidget *entry2;
    GtkWidget *result_label;
} AppWidgets;

// 더하기 버튼 클릭 콜백
static void on_add_clicked(GtkButton *button, gpointer user_data) {
    AppWidgets *widgets = (AppWidgets *)user_data;

    const char *text1 = gtk_entry_get_text(GTK_ENTRY(widgets->entry1));
    const char *text2 = gtk_entry_get_text(GTK_ENTRY(widgets->entry2));

    double a = g_ascii_strtod(text1, NULL);
    double b = g_ascii_strtod(text2, NULL);
    double res = a + b;

    char buf[64];
    g_snprintf(buf, sizeof(buf), "결과: %.2f", res);
    gtk_label_set_text(GTK_LABEL(widgets->result_label), buf);
}

// 빼기 버튼 클릭 콜백
static void on_sub_clicked(GtkButton *button, gpointer user_data) {
    AppWidgets *widgets = (AppWidgets *)user_data;

    const char *text1 = gtk_entry_get_text(GTK_ENTRY(widgets->entry1));
    const char *text2 = gtk_entry_get_text(GTK_ENTRY(widgets->entry2));

    double a = g_ascii_strtod(text1, NULL);
    double b = g_ascii_strtod(text2, NULL);
    double res = a - b;

    char buf[64];
    g_snprintf(buf, sizeof(buf), "결과: %.2f", res);
    gtk_label_set_text(GTK_LABEL(widgets->result_label), buf);
}

// 곱하기 버튼 클릭 콜백
static void on_mul_clicked(GtkButton *button, gpointer user_data) {
    AppWidgets *widgets = (AppWidgets *)user_data;

    const char *text1 = gtk_entry_get_text(GTK_ENTRY(widgets->entry1));
    const char *text2 = gtk_entry_get_text(GTK_ENTRY(widgets->entry2));

    double a = g_ascii_strtod(text1, NULL);
    double b = g_ascii_strtod(text2, NULL);
    double res = a * b;

    char buf[64];
    g_snprintf(buf, sizeof(buf), "결과: %.2f", res);
    gtk_label_set_text(GTK_LABEL(widgets->result_label), buf);
}

// 나누기 버튼 클릭 콜백
static void on_div_clicked(GtkButton *button, gpointer user_data) {
    AppWidgets *widgets = (AppWidgets *)user_data;

    const char *text1 = gtk_entry_get_text(GTK_ENTRY(widgets->entry1));
    const char *text2 = gtk_entry_get_text(GTK_ENTRY(widgets->entry2));

    double a = g_ascii_strtod(text1, NULL);
    double b = g_ascii_strtod(text2, NULL);

    char buf[64];
    if (b == 0) {
        g_snprintf(buf, sizeof(buf), "오류: 0으로 나눌 수 없음");
    } else {
        double res = a / b;
        g_snprintf(buf, sizeof(buf), "결과: %.2f", res);
    }

    gtk_label_set_text(GTK_LABEL(widgets->result_label), buf);
}

int main(int argc, char *argv[]) {
    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *label1, *label2;
    GtkWidget *entry1, *entry2;
    GtkWidget *add_button, *sub_button, *mul_button, *div_button;
    GtkWidget *result_label;

    AppWidgets widgets;

    gtk_init(&argc, &argv);

    // 1. 윈도우 생성
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "GTK 계산기 (4칙연산)");
    gtk_window_set_default_size(GTK_WINDOW(window), 350, 180);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // 2. Grid 레이아웃 생성
    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 5);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 10);

    // 3. 위젯 생성
    label1 = gtk_label_new("숫자 1:");
    label2 = gtk_label_new("숫자 2:");

    entry1 = gtk_entry_new();
    entry2 = gtk_entry_new();

    add_button = gtk_button_new_with_label("더하기");
    sub_button = gtk_button_new_with_label("빼기");
    mul_button = gtk_button_new_with_label("곱하기");
    div_button = gtk_button_new_with_label("나누기");

    result_label = gtk_label_new("결과:");

    // 구조체에 저장
    widgets.entry1 = entry1;
    widgets.entry2 = entry2;
    widgets.result_label = result_label;

    // 4. Grid 배치
    gtk_grid_attach(GTK_GRID(grid), label1,  0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry1,  1, 0, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), label2,  0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry2,  1, 1, 2, 1);

    gtk_grid_attach(GTK_GRID(grid), add_button, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), sub_button, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), mul_button, 2, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), div_button, 3, 2, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), result_label, 0, 3, 4, 1);

    // 5. 시그널 연결
    g_signal_connect(add_button, "clicked", G_CALLBACK(on_add_clicked), &widgets);
    g_signal_connect(sub_button, "clicked", G_CALLBACK(on_sub_clicked), &widgets);
    g_signal_connect(mul_button, "clicked", G_CALLBACK(on_mul_clicked), &widgets);
    g_signal_connect(div_button, "clicked", G_CALLBACK(on_div_clicked), &widgets);

    // 6. 출력
    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
